[%raw "require('isomorphic-fetch')"];

[@bs.val] external decodeURIComponent: string => string = "";

[@bs.deriving {jsConverter: newType}]
type imageQuery = {url: option(string)};

[@bs.val] external cacheTTL: string = "process.env.CACHE_TTL";
[@bs.val] external cacheMaxSize: string = "process.env.CACHE_MAX_SIZE";
[@bs.val] external whitelistUrl: option(string) = "process.env.WHITELIST_URL";

let diskCache =
  CacheManager.caching({
    "store": CacheManager.fsStore,
    "options": {
      ttl: int_of_string(cacheTTL),
      maxsize: int_of_string(cacheMaxSize),
      path: "/diskcache",
      preventfill: true,
      reviveBuffers: true,
    },
  });

let respondScreenshot = (url, response) =>
  Js.Promise.(
    diskCache##wrap(
      url,
      cacheCallback =>
        Screenshot.takeScreenshotDefault(url)
        |> then_(imageBuffer => {
             cacheCallback(Js.Nullable.null, {
                                               "binary": {
                                                 image: imageBuffer,
                                               },
                                             });
             resolve();
           })
        |> catch(_ => {
             ConnectServer.setStatusCode(response, 500);
             ConnectServer.callVariantResponse1(response);
             resolve();
           })
        |> ignore,
      (_, result) => ConnectServer.callVariantResponse3(response, result##binary##image, "binary"),
    )
  );

type urlContainerObject = {sourceUrl: string};

let decodeUrlsStructure = (list, url) => {
  let decodeUrlObj = input: urlContainerObject => Json.Decode.{sourceUrl: input |> field("sourceUrl", string)};
  let decodeUrls = list => list |> Json.Decode.array(decodeUrlObj);
  let decodedUrls = decodeUrls(list);
  decodedUrls |> Array.to_list |> List.map(re => re.sourceUrl) |> List.find(sourceUrl => url === sourceUrl);
};

let screenshotOrErrorResponse = (url, response) =>
  switch (respondScreenshot(url, response)) {
  | exception (Js.Exn.Error(_)) =>
    ConnectServer.setStatusCode(response, 500);
    ConnectServer.callVariantResponse1(response);
  | _ => ignore()
  };

let handler = (request, response: ConnectServer.response) => {
  let queryParams = imageQueryFromJs(request##query);
  switch (queryParams.url) {
  | None =>
    ConnectServer.setStatusCode(response, 400);
    ConnectServer.callVariantResponse1(response);
  | Some(url) =>
    Js.Promise.(
      switch (whitelistUrl) {
      | None =>
        screenshotOrErrorResponse(url, response);
        ignore();
      | Some(whitelistUrl) =>
        whitelistUrl
        |> decodeURIComponent
        |> Fetch.fetch
        |> then_(Fetch.Response.json)
        |> then_(list => {
             switch (decodeUrlsStructure(list, url)) {
             | exception Not_found =>
               ConnectServer.setStatusCode(response, 400);
               ConnectServer.callVariantResponse2(response, "not on whitelist");
               ignore();
             | _ =>
               screenshotOrErrorResponse(url, response);
               ignore();
             };
             resolve();
           })
        |> catch(_ => {
             ConnectServer.setStatusCode(response, 500);
             ConnectServer.callVariantResponse2(
               response,
               "Could not use whitelist. make sure the whitelist url is correct and data structure returned is array.",
             );
             resolve();
           })
        |> ignore
      }
    )
  };
};
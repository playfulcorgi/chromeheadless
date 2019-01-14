[%raw "require('isomorphic-fetch')"];

[@bs.val] external decodeURIComponent: string => string = "";

[@bs.deriving {jsConverter: newType}]
type imageQuery = {url: option(string)};

[@bs.val] external imageCacheTTL: string = "process.env.IMAGE_CACHE_TTL";
[@bs.val] external imageCacheMaxSize: string = "process.env.IMAGE_CACHE_MAX_SIZE";
[@bs.val] external whitelistUrl: option(string) = "process.env.WHITELIST_URL";
[@bs.val] external whitelistCacheTTL: option(string) = "process.env.WHITELIST_CACHE_TTL";

let diskCache =
  CacheManager.caching({
    "store": CacheManager.fsStore,
    "options": {
      ttl: int_of_string(imageCacheTTL),
      maxsize: int_of_string(imageCacheMaxSize),
      path: "/diskcache",
      preventfill: true,
      reviveBuffers: true,
    },
  });

let memoryCache = CacheManager.caching({"store": "memory", "ttl": whitelistCacheTTL});

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

let decodeUrlsStructure = list => {
  let decodeUrlObj = input: urlContainerObject => Json.Decode.{sourceUrl: input |> field("sourceUrl", string)};
  let decodeUrls = list => list |> Json.Decode.array(decodeUrlObj);
  let decodedUrls = decodeUrls(list);
  decodedUrls |> Array.to_list;
};

let isWhitelisted = (list: list(urlContainerObject), url: string) =>
  list |> List.map(re => re.sourceUrl) |> List.find(sourceUrl => url === sourceUrl);

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
  | Some(requestedUrl) =>
    Js.Promise.(
      switch (whitelistUrl) {
      | None =>
        screenshotOrErrorResponse(requestedUrl, response);
        ignore();
      | Some(whitelistUrl) =>
        memoryCache##wrap(whitelistUrl, () =>
          whitelistUrl |> decodeURIComponent |> Fetch.fetch |> then_(Fetch.Response.json)
        )
        |> then_(jsonList => {
             let decodedListStructure = jsonList->decodeUrlsStructure;
             let isWhitelistedResult = decodedListStructure->isWhitelisted(requestedUrl);
             switch (isWhitelistedResult) {
             | exception Not_found =>
               ConnectServer.setStatusCode(response, 400);
               ConnectServer.callVariantResponse2(response, "not on whitelist");
               resolve();
             | _ =>
               screenshotOrErrorResponse(requestedUrl, response);
               resolve();
             };
           })
        |> catch(error => {
             Js.log2("error", error);
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
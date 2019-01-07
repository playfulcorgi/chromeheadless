[@bs.deriving {jsConverter: newType}]
type imageQuery = {
  url: option(string),
  password: option(string)
};

[@bs.val] external cacheTTL:string = "process.env.CACHE_TTL";
[@bs.val] external cacheMaxSize:string = "process.env.CACHE_MAX_SIZE";

let diskCache = CacheManager.caching([%bs.obj {
  store: CacheManager.fsStore,
  options: {
    ttl: int_of_string(cacheTTL),
    maxsize: int_of_string(cacheMaxSize),
    path: "/diskcache",
    preventfill:true,
    reviveBuffers: true
  }
}])


let respondScreenshot = (url, response) => {
  Js.Promise.(
    diskCache##wrap(
      url, 
      (cacheCallback) => {
        Screenshot.takeScreenshotDefault(url)
          |> then_(imageBuffer => {
            cacheCallback(
              Js.Nullable.null, 
              [%bs.obj {
                binary: { image: imageBuffer }
              }]
            )
            resolve()
          })
          |> catch(_ => {
            ConnectServer.setStatusCode(response, 500)
            ConnectServer.callVariantResponse1(response)
            resolve()
          })
          |> ignore
      },
      (_, result) => {
        ConnectServer.callVariantResponse3(response, result##binary##image, "binary")
      }
    )
  )
};

let handler = (request, response: ConnectServer.response) => {
  let queryParams = imageQueryFromJs(request##query)
  switch (queryParams.url) {
    | None => {
      ConnectServer.setStatusCode(response, 400)
      ConnectServer.callVariantResponse1(response)
      None
    }
    | Some(url) => {
      try(respondScreenshot(url, response)){
        | Js.Exn.Error(_) => {
          ConnectServer.setStatusCode(response, 500)
          ConnectServer.callVariantResponse1(response)
        }
      }
      None
    }
  };
};

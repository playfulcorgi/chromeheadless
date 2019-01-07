module P = Js.Promise
module CH = ChromeHeadless

exception ThumbnailNotCreated(unit);

let takeScreenshot = (url: string, fullPage: bool, width: int, height: int) =>
  CH.launch(CH.launchOptions(~args=[|
    "--no-sandbox",
    "--disable-gpu"
  |]))
  |> P.then_(browser => 
    CH.newPage(browser)
    |> P.then_(page =>
      CH.pageSetViewport(CH.pageSetViewportOptions(~width, ~height, ~isMobile=true, ~deviceScaleFactor=2), page)
      |> P.then_(() => CH.pageGoto(url, page))
      |> P.then_(() => {
        P.make((~resolve, ~reject as _) => {
          Js.Global.setTimeout(() => {
            resolve(. None);
          }, 2000)
          |> ignore
        })
      })
      |> P.then_((_) => CH.pageScreenshot(CH.pageScreenshotOptions(~path=None, ~fullPage), page))
      |> P.then_((buffer) => {
        CH.browserClose(browser)
          |> P.then_(() => P.resolve(buffer))
      }
      )
    )
  )
  |> P.catch(error => {
    Js.log2("Error creating thumbnail", error)
    P.reject(ThumbnailNotCreated())
  })

let takeScreenshotDefault = (url) => takeScreenshot(url, false, 480, 480);

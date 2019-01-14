module P = Js.Promise;
module CH = ChromeHeadless;

exception ThumbnailNotCreated(unit);

let maybeBrowserPromise: ref(option(Js.Promise.t(CH.browser))) = ref(None);

let getBrowser = () =>
  switch (maybeBrowserPromise^) {
  | None =>
    let newBrowserPromise = CH.launch(CH.launchOptions(~args=[|"--no-sandbox", "--disable-gpu"|]));
    let newBrowserInstance = Some(newBrowserPromise);
    maybeBrowserPromise := newBrowserInstance;
    newBrowserPromise;
  | Some(browserPromise) => browserPromise
  };

let takeScreenshot = (url: string, fullPage: bool, width: int, height: int) =>
  getBrowser()
  |> P.then_(browser =>
       CH.newPage(browser)
       |> P.then_((page: CH.page) =>
            CH.pageSetViewport(
              CH.pageSetViewportOptions(~width, ~height, ~isMobile=true, ~deviceScaleFactor=2),
              page,
            )
            |> P.then_(() => CH.pageGoto(url, page))
            |> P.then_(() =>
                 P.make((~resolve, ~reject as _) => Js.Global.setTimeout(() => resolve(. None), 2000) |> ignore)
               )
            |> P.then_(_ => CH.pageScreenshot(CH.pageScreenshotOptions(~path=None, ~fullPage), page))
            |> P.then_((buffer) => {
              CH.pageClose(page)
              |> P.then_(() => P.resolve(buffer))
            })
          )
     )
  |> P.catch(error => {
       Js.log2("Error creating thumbnail", error);
       P.reject(ThumbnailNotCreated());
     });

let takeScreenshotDefault = url => takeScreenshot(url, false, 480, 480);
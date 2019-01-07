[@bs.deriving abstract]
type launchArgs = { args: array(string) };
type page = { goto: string => Js.Promise.t(page) };
type browser = { newPage: unit => Js.Promise.t(page) };
[@bs.deriving abstract]
type pageScreenshotArgs = {
  path: option(string)
};

[@bs.module "puppeteer"] external launch: launchArgs => Js.Promise.t(browser) = "launch";
[@bs.send] external newPage: browser => Js.Promise.t(page) = "";
[@bs.send.pipe: page] external pageGoto: string => Js.Promise.t('response) = "goto";
[@bs.send.pipe: page] external pageScreenshot: pageScreenshotArgs => Js.Promise.t('response) = "screenshot";
[@bs.send] external browserClose: browser => Js.Promise.t(unit) = "close";


[@bs.deriving abstract]
type pageSetViewportArgs = {
  width: int,
  height: int
};
[@bs.deriving abstract]
type asciifyArgs = {
  fit: string,
  width: int,
  height: int
};
[@bs.module] external asciify: (Buffer.t, asciifyArgs) => Js.Promise.t(string) = "asciify-image";
[@bs.send.pipe: page] external pageSetViewport: pageSetViewportArgs => Js.Promise.t(unit) = "setViewport";


Js.Promise.(
  launch(launchArgs(~args=[|
    "--no-sandbox",
    "--disable-gpu"
  |]))
  |> then_(browser => {
    newPage(browser)
    |> then_(page =>
      pageSetViewport(pageSetViewportArgs(~width=640, ~height=640), page)
        |> then_(() => pageGoto("https://via.placeholder.com/300", page))
        |> then_(() => pageScreenshot(pageScreenshotArgs(~path=None), page))
        |> then_(buffer => asciify(buffer, asciifyArgs(~fit="box", ~width=100, ~height=100)))
        |> then_((asciified) => {
          Js.log(asciified)
          resolve()
        })
    )
    |> then_(() => browserClose(browser))
  })
  |> catch(error => {
    Js.log(error)
    resolve()
  })
);

[@bs.deriving abstract]
type launchOptions = {
  args: array(string)
};
type page = { goto: string => Js.Promise.t(page) };
type browser = { newPage: unit => Js.Promise.t(page) };
[@bs.deriving abstract]
type pageSetViewportOptions = {
  width: int,
  height: int,
  isMobile: bool,
  deviceScaleFactor: int
};
[@bs.deriving abstract]
type pageScreenshotOptions = {
  path: option(string),
  fullPage: bool
};

[@bs.module "puppeteer"] external launch: launchOptions => Js.Promise.t(browser) = "launch";
[@bs.send] external newPage: browser => Js.Promise.t(page) = "";
[@bs.send.pipe: page] external pageGoto: string => Js.Promise.t('response) = "goto";
[@bs.send.pipe: page] external pageScreenshot: pageScreenshotOptions => Js.Promise.t('response) = "screenshot";
[@bs.send] external browserClose: browser => Js.Promise.t(unit) = "close";
[@bs.send.pipe: page] external pageSetViewport: pageSetViewportOptions => Js.Promise.t(unit) = "setViewport";
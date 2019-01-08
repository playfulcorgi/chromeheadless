FROM playfulcorgi/faas-node:0.10.0

RUN apt-get update
RUN apt-get install -y apt-transport-https ca-certificates curl gnupg
RUN curl -sSL https://dl.google.com/linux/linux_signing_key.pub | apt-key add -
RUN echo "deb https://dl.google.com/linux/chrome/deb/ stable main" > /etc/apt/sources.list.d/google-chrome.list
RUN apt-get update
# When using Puppeteer, Chrome headless works in Docker nearly out of the box with a few extra flags and installing a few apt packages. Trying to
# install Chromium or Chrome separately in Docker proved more difficult.
RUN apt-get install -y google-chrome-stable
RUN adduser --disabled-password --gecos '' chromeheadless

RUN apt-get install -y ocaml-nox

COPY . ./
RUN yarn config set cache-folder /root/yarn-cache
RUN yarn install
RUN yarn run build

ENV HANDLER_FILE_SUBPATH="src/handler.js"

RUN mkdir -p /diskcache
RUN chown -R chromeheadless /diskcache
RUN chmod 755 /diskcache
USER chromeheadless
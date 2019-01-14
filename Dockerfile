FROM playfulcorgi/faas-node:0.10.0

RUN apt-get update
RUN apt-get install -y apt-transport-https ca-certificates curl gnupg
RUN curl -sSL https://dl.google.com/linux/linux_signing_key.pub | apt-key add -
RUN echo "deb https://dl.google.com/linux/chrome/deb/ stable main" > /etc/apt/sources.list.d/google-chrome.list
RUN apt-get update
# When using Puppeteer, Chrome headless works in Docker nearly out of the box with a few extra flags and installing a few apt packages. Trying to
# install Chromium or Chrome separately in Docker proved more difficult.
RUN apt-get install -y google-chrome-stable
# the following RUN solves https://github.com/GoogleChrome/puppeteer/issues/404:
RUN apt-get install -y libpangocairo-1.0-0 libx11-xcb1 libxcomposite1 libxcursor1 libxdamage1 libxi6 libxtst6 libnss3 libcups2 libxss1 libxrandr2 \
  libgconf2-4 libasound2 libatk1.0-0 libgtk-3-0
RUN adduser --disabled-password --gecos '' chromeheadless

COPY . ./
RUN yarn config set cache-folder /root/yarn-cache
RUN yarn install
RUN yarn run build

ENV HANDLER_FILE_SUBPATH="src/handler.js"

RUN mkdir -p /diskcache
RUN chown -R chromeheadless /diskcache
RUN chmod 755 /diskcache
USER chromeheadless
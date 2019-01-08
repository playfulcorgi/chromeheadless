// Special watch for Reason when sending files via ssh for "yarn build". Otherwise file is built before being fully uploaded.
const chokidar = require('chokidar')
const path = require('path')
const debounce = require('lodash/debounce')
const { spawn } = require('child_process')

const fileSizeChangeWait = 400

const rebuild = () => {
  spawn('yarn', ['run', 'build'], {stdio: "inherit"})
}

const debounceGracePeriod = 50

const debouncedRebuild = debounce(rebuild, fileSizeChangeWait + debounceGracePeriod, { leading: false, trailing: true })

chokidar.watch(path.join(__dirname, './src/**/*.re'), {
  awaitWriteFinish: {
    stabilityThreshold: fileSizeChangeWait,
    pollInterval: 50
  }
})
  .on('all', debouncedRebuild)

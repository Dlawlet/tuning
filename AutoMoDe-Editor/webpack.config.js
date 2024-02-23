/* eslint-disable no-undef */
const path = require("path");

module.exports = env => {
    const isProduction = env && env.production;
    return {
        mode: isProduction ? "production" : "development",
        entry: "./src/index.js",
        resolve: {
            alias: {Config: path.resolve(__dirname, "config")}
        },
        output: {
            filename: "main.js",
            path: path.resolve(__dirname, "dist"),
            publicPath: '/'
        },
        devtool: isProduction ? false : "cheap-module-eval-source-map",
        module: {
            rules: [
                { test: /\.jsx$/, exclude: /node_modules/, loader: "babel-loader" }
            ]
        }
    };
};
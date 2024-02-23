module.exports = {
    env: {
        "browser": true,
        "es6": true
    },
    extends: "eslint:recommended",
    globals: {
        "Atomics": "readonly",
        "SharedArrayBuffer": "readonly"
    },
    parserOptions: {
        "ecmaVersion": 2018,
        "sourceType": "module",
        "ecmaFeatures": {
            "jsx": true
        }
    },
    rules: {
        "indent": [
            "warn",
            4
        ],
        "linebreak-style": [
            "warn",
            "windows"
        ],
        "quotes": [
            "warn",
            "double"
        ],
        "semi": [
            "warn",
            "always"
        ],
        "no-var": [
            "error"
        ],
        "eqeqeq": [
            "error",
            "smart"
        ]
    }
};
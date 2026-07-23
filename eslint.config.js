module.exports = [
    {
        ignores: ['build/**', 'build_*/**']
    },
    {
        files: ['**/*.js', '**/*.ts'],
        languageOptions: {
            ecmaVersion: 2022,
            sourceType: 'commonjs',
            globals: {
                console: 'readonly',
                process: 'readonly',
                __dirname: 'readonly',
                require: 'readonly',
                Math: 'readonly',
                setTimeout: 'readonly',
                performance: 'readonly',
                Date: 'readonly',
                window: 'readonly',
                document: 'readonly',
                module: 'readonly'
            }
        },
        rules: {
            'semi': ['error', 'always'],
            'quotes': ['error', 'single']
        }
    }
];

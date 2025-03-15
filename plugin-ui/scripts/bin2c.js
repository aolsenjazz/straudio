import fs from 'fs';
import path from 'path';

// 1) Hard-coded input, output, and final destination
const INPUT_HTML = path.join('dist', 'index.html');

const CPP_FILENAME = 'PluginUI.cpp';
const HPP_FILENAME = 'PluginUI.hpp';

const DIST_OUTPUT_CPP = path.join('dist', CPP_FILENAME);
const DIST_OUTPUT_HPP = path.join('dist', HPP_FILENAME);

// Destination folder for final files
const FINAL_DEST_DIR = path.join('..', 'plugin', 'Straudio', 'src', 'PluginUI');

// 2) Read the input HTML as text (assuming UTF-8)
const htmlText = fs.readFileSync(INPUT_HTML, 'utf8');

// 3) Use JSON.stringify to properly escape the HTML content as a C string literal
// JSON.stringify returns a quoted string, so we can use it directly.
const escapedHtml = JSON.stringify(htmlText);

// 4) Build the C++ header and source content
const arrayName = 'PLUGIN_UI';
const lengthName = 'PLUGIN_UI_length';

const headerFileContents = `#pragma once

#include <stdint.h>

// Declarations for the embedded HTML data as a C string.
extern const char ${arrayName}[];
extern const int ${lengthName};
`;

const cppFileContents = `#include "${HPP_FILENAME}"

const char ${arrayName}[] = ${escapedHtml};

const int ${lengthName} = ${htmlText.length};
`;

// 5) Write the generated files inside dist/
fs.writeFileSync(DIST_OUTPUT_HPP, headerFileContents);
fs.writeFileSync(DIST_OUTPUT_CPP, cppFileContents);

// why is eslint complaining about this 🙃
// eslint-disable-next-line no-undef
console.log(`Wrote \${CPP_FILENAME} and \${HPP_FILENAME} to dist/`);

// 6) Move the generated files from dist/ to ../plugin/Straudio/src/PluginUI/
fs.mkdirSync(FINAL_DEST_DIR, { recursive: true }); // ensure destination exists

const finalCppPath = path.join(FINAL_DEST_DIR, CPP_FILENAME);
const finalHppPath = path.join(FINAL_DEST_DIR, HPP_FILENAME);

fs.renameSync(DIST_OUTPUT_CPP, finalCppPath);
fs.renameSync(DIST_OUTPUT_HPP, finalHppPath);

// why is eslint complaining about this 🙃
// eslint-disable-next-line no-undef
console.log(
  `Moved \${CPP_FILENAME} and \${HPP_FILENAME} to \${FINAL_DEST_DIR}`
);

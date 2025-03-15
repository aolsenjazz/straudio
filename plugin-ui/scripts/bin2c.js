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

// 2) Read the input HTML as binary data (Buffer)
const dataBuffer = fs.readFileSync(INPUT_HTML);
const dataLength = dataBuffer.length;

// 3) Build the C++ header content.
// This declares the embedded HTML data as a byte array.
const arrayName = 'PLUGIN_UI';
const lengthName = 'PLUGIN_UI_length';

const headerFileContents = `#pragma once

#include <stdint.h>

// Declarations for the embedded HTML data as a byte array.
extern const uint8_t ${arrayName}[${dataLength}];
extern const int ${lengthName};
`;

// 4) Build the C++ source content.
// We convert the binary data into a comma-separated list of byte values.
const byteArray = [...dataBuffer].join(',');
const cppFileContents = `#include "${HPP_FILENAME}"

const uint8_t ${arrayName}[${dataLength}] = {
  ${byteArray}
};

const int ${lengthName} = ${dataLength};
`;

// 5) Write the generated files inside the dist/ directory
fs.writeFileSync(DIST_OUTPUT_HPP, headerFileContents);
fs.writeFileSync(DIST_OUTPUT_CPP, cppFileContents);

console.log(`Wrote ${CPP_FILENAME} and ${HPP_FILENAME} to dist/`);

// 6) Move the generated files from dist/ to the final destination directory
fs.mkdirSync(FINAL_DEST_DIR, { recursive: true }); // ensure destination exists

const finalCppPath = path.join(FINAL_DEST_DIR, CPP_FILENAME);
const finalHppPath = path.join(FINAL_DEST_DIR, HPP_FILENAME);

fs.renameSync(DIST_OUTPUT_CPP, finalCppPath);
fs.renameSync(DIST_OUTPUT_HPP, finalHppPath);

console.log(`Moved ${CPP_FILENAME} and ${HPP_FILENAME} to ${FINAL_DEST_DIR}`);

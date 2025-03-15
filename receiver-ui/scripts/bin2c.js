import fs from 'fs';
import path from 'path';

// 1) Hard-coded input, output, and final destination
const INPUT_HTML = path.join('dist', 'index.html');

const CPP_FILENAME = 'ReceiverUI.cpp';
const HPP_FILENAME = 'ReceiverUI.hpp';

const DIST_OUTPUT_CPP = path.join('dist', CPP_FILENAME);
const DIST_OUTPUT_HPP = path.join('dist', HPP_FILENAME);

// Destination folder for final files
const FINAL_DEST_DIR = path.join(
  '..',
  'plugin',
  'Straudio',
  'src',
  'ReceiverUI'
);

// 2) Read the input HTML
const data = fs.readFileSync(INPUT_HTML);

// 3) Build the C++ header and source content
const arrayName = 'RECEIVER_UI';
const lengthName = 'RECEIVER_UI_length';

const headerFileContents = `#pragma once

#include <stdint.h>

#ifndef RESOURCE_T_DEFINED
#define RESOURCE_T_DEFINED
struct resource_t {
  resource_t(const char* name, const uint8_t* data, uint32_t size) : name(name), data(data), size(size) {}
  const char* name; const uint8_t* data; const uint32_t size;
};
#endif

// Declarations for the embedded HTML data
extern const uint8_t ${arrayName}[${data.length}];
extern const int ${lengthName};
`;

const cppFileContents = `#include "${HPP_FILENAME}"

const uint8_t ${arrayName}[${data.length}] = {
    ${[...data].join(',')}
};

const int ${lengthName} = ${data.length};
`;

// 4) Write them inside dist/
fs.writeFileSync(DIST_OUTPUT_HPP, headerFileContents);
fs.writeFileSync(DIST_OUTPUT_CPP, cppFileContents);

// why is eslint complaining about this 🙃
// eslint-disable-next-line no-undef
console.log(`Wrote ${CPP_FILENAME} and ${HPP_FILENAME} to dist/`);

// 5) Move the generated files from dist/ to ../plugin/Straudio/src/PluginUI/
fs.mkdirSync(FINAL_DEST_DIR, { recursive: true }); // ensure destination exists

const finalCppPath = path.join(FINAL_DEST_DIR, CPP_FILENAME);
const finalHppPath = path.join(FINAL_DEST_DIR, HPP_FILENAME);

fs.renameSync(DIST_OUTPUT_CPP, finalCppPath);
fs.renameSync(DIST_OUTPUT_HPP, finalHppPath);

// why is eslint complaining about this 🙃
// eslint-disable-next-line no-undef
console.log(`Moved ${CPP_FILENAME} and ${HPP_FILENAME} to ${FINAL_DEST_DIR}`);

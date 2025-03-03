/* eslint-disable no-var */

declare global {
  // Declare IPlugSendMsg as a global function
  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  function IPlugSendMsg(message: any): void;

  // SendParameterValueFromUI
  var SPVFUI: (paramIdx: number, value: number) => void;

  // BeginInformHostOfParamChangeFromUI
  var BPCFUI: (paramIdx: number) => void;

  // EndInformHostOfParamChangeFromUI
  var EPCFUI: (paramIdx: number) => void;

  // SendParameterValueFromDelegate
  var SPVFD: (paramIdx: number, value: number) => void;

  // SendControlValueFromDelegate
  var SCVFD: (ctrlTag: number, value: number) => void;

  // SendControlMsgFromDelegate
  var SCMFD: (
    ctrlTag: number,
    msgTag: number,
    dataSize: number,
    msg: string
  ) => void;

  // SendArbitraryMsgFromDelegate
  var SAMFD: (msgTag: number, dataSize: number, msg: string) => void;

  var OnParamChange: (paramIdx: number, value: number) => void;
}

export {};

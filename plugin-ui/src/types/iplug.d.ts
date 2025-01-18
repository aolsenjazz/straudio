declare global {
  // Declare IPlugSendMsg as a global function
  function IPlugSendMsg(message: any): void;

  interface Window {
    // SendParameterValueFromUI
    SPVFUI: (paramIdx: number, value: number) => void;

    // BeginInformHostOfParamChangeFromUI
    BPCFUI: (paramIdx: number) => void;

    // EndInformHostOfParamChangeFromUI
    EPCFUI: (paramIdx: number) => void;
    
    // SendParameterValueFromDelegate
    SPVFD: (paramIdx: number, value: number) => void;

    // SendControlValueFromDelegate
    SCVFD: (ctrlTag: number, value: number) => void;

    // SendControlMsgFromDelegate
    SCMFD: (ctrlTag: number, msgTag: number, dataSize: number, msg: string) => void;

    // SendArbitraryMsgFromDelegate
    SAMFD: (msgTag: number, dataSize: number, msg: string) => void;
    
    OnParamChange: (paramIdx: number, value: number) => void;
  }
}

export {}; 

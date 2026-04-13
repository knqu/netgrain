import "../styling/CodeEditor.css"
import * as monaco from 'monaco-editor';
import React, { useEffect, useRef } from "react";



// function test() {
//     //const val = "test";
//     const myEditor = monaco.editor.create(DocumentTimeline.getElementById("tester"), {
//         language: "python",
//         automaticLayout: true,
//     });
// }
monaco.languages.registerCompletionItemProvider('python', {
    provideCompletionItems: (model, position) => {
      console.log(model)
        const suggestions = [
            {
                label: 'get_balance',
                kind: monaco.languages.CompletionItemKind.Function,
                insertText: 'get_balance()',
                insertTextRules: monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                detail: 'get the balance present in the simulator',
                range: {
                    startLineNumber: position.lineNumber,
                    endLineNumber: position.lineNumber,
                    startColumn: position.column - 1,
                    endColumn: position.column
                }
            },
            {
                label: 'place_order',
                kind: monaco.languages.CompletionItemKind.Function,
                insertText: 'place_order(ticker, quantity, target_price, side, type)',
                insertTextRules: monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                detail: 'Place a stock order',
                range: {
                    startLineNumber: position.lineNumber,
                    endLineNumber: position.lineNumber,
                    startColumn: position.column - 1,
                    endColumn: position.column
                }
            },
            {
                label: 'cancel_order',
                kind: monaco.languages.CompletionItemKind.Function,
                insertText: 'cancel_order(order_id)',
                insertTextRules: monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                detail: 'cancel a stock order',
                range: {
                    startLineNumber: position.lineNumber,
                    endLineNumber: position.lineNumber,
                    startColumn: position.column - 1,
                    endColumn: position.column
                }
            },
            {
                label: 'calculate_fee',
                kind: monaco.languages.CompletionItemKind.Function,
                insertText: 'calculate_fee(config, fill_quantity, trade_value)',
                insertTextRules: monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                detail: 'calculate fee for a stock order',
                range: {
                    startLineNumber: position.lineNumber,
                    endLineNumber: position.lineNumber,
                    startColumn: position.column - 1,
                    endColumn: position.column
                }
            },
            {
                label: 'get_config',
                kind: monaco.languages.CompletionItemKind.Function,
                insertText: 'get_config(ticker)',
                insertTextRules: monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                detail: 'get the config for a stock ticker',
                range: {
                    startLineNumber: position.lineNumber,
                    endLineNumber: position.lineNumber,
                    startColumn: position.column - 1,
                    endColumn: position.column
                }
            },
        ];
        
        return { suggestions: suggestions };
    }
});

const EditorContainer: React.FC = () => {
    const containerRef = useRef<HTMLDivElement | null>(null);
    const myEditor = useRef<monaco.editor.IStandaloneCodeEditor | null>(null);

    useEffect(() => {
        if (!containerRef.current) return;
        // const value = `this is a test`;
        myEditor.current = monaco.editor.create(containerRef.current, {
            //value,
            language: "python",
            automaticLayout: false,
        });


        // ensure that layout operates correctly
        const resizeObserver = new ResizeObserver(() => {
            window.requestAnimationFrame(() => {
                if (containerRef.current) {
                    myEditor.current?.layout();
                }
            });
        });    

        resizeObserver.observe(containerRef.current);
            
        setTimeout(() => {
            myEditor.current?.layout();
        }, 0);

        return () => {
            if (myEditor.current) {
                resizeObserver.disconnect();
                myEditor.current.dispose();
            }
        };
    }, []);

        return <div className="EditorContainer" ref={containerRef}/>
};


export default function CodeEditor() {
    return (
        // <h1>
        //      This is the code editing page.
        // </h1>
        <div className="editor_outer" >
            <div className="editor_inner">
                
                <div className="code_editor">
                    <EditorContainer></EditorContainer>
                </div>
            </div>
        </div>
        
    )};

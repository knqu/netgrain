import * as monaco from 'monaco-editor';
import React, { useEffect, useRef } from "react";



// function test() {
//     //const val = "test";
//     const myEditor = monaco.editor.create(DocumentTimeline.getElementById("tester"), {
//         language: "python",
//         automaticLayout: true,
//     });
// }


const Test: React.FC = () => {
    const containerRef = useRef<HTMLDivElement | null>(null);
    const myEditor = useRef<monaco.editor.IStandaloneCodeEditor | null>(null);

    useEffect(() => {
        if (!containerRef.current) return;
        myEditor.current = monaco.editor.create(containerRef.current, {
            language: "python",
            automaticLayout: true,
        });


            

            
        return () => {
            if (myEditor.current) {
                myEditor.current.dispose();
            }
        };
    }, []);

        return <div className="tester" ref={containerRef} style={{height: "90vh", width: "90%"}}/>
};


export default function CodeEditor() {
    return (
        // <h1>
        //      This is the code editing page.
        // </h1>
        <div className="editor_outer" style={{height: "100%", width: "100vh"}}>
            <div className="editor_inner">
                <div className="editor" >
                    <Test></Test>
                </div>
            </div>
        </div>
        
    )};

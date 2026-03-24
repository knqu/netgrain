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
                <div className="editor">
                    <EditorContainer></EditorContainer>
                </div>
            </div>
        </div>
        
    )};

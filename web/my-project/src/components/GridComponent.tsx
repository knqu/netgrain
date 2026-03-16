import { useState, useEffect } from 'react';
import { Responsive, useContainerWidth } from 'react-grid-layout';

interface GridProps {
  widgets: number[];
  removeWidget: (id: number) => void;
}

export default function GridComponent({ widgets, removeWidget }: GridProps) {
  const { width, containerRef, mounted } = useContainerWidth();
  const [layouts, setLayouts] = useState<any>({});

  useEffect(() => {
    const loadSavedLayout = async () => {
      try {
        const response = await fetch('/api/fetchLayout');
        if (response.ok) {
          const savedData = await response.json();
          if (Object.keys(savedData).length > 0) {
            setLayouts(savedData);
          }
        }
      } catch (err) {
        console.error("Failed to load layout from server", err);
      }
    };

    loadSavedLayout();
  }, []);

  const saveLayoutToServer = async () => {
    try {
      const response = await fetch('/api/saveLayout', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(layouts)
      });
      
      if (response.ok) {
        alert("Dashboard layout saved successfully!");
      } else {
        alert("Failed to save layout.");
      }
    } catch (err) {
      console.error("Error saving layout", err);
    }
  };

  const onLayoutChange = (current: any, all: any) => {
    setLayouts(all);
    console.log(current);
  };

  return (
    <div className="flex flex-col flex-1 w-full">
    <div className="flex justify-end p-4">
        <button 
          onClick={saveLayoutToServer}
          className="bg-blue-600 hover:bg-blue-500 text-white font-bold py-2 px-4 rounded shadow-lg transition-colors"
        >
          Save Layout
        </button>
      </div>
      <div ref={containerRef} className="Grid">
      {mounted && (
        <Responsive
        layouts={layouts}
        onLayoutChange={onLayoutChange}
        breakpoints={{ lg: 1100, md: 996, sm: 768, xs: 480, xxs: 0 }}
        cols={{ lg: 12, md: 12, sm: 6, xs: 4, xxs: 2 }}
        width={width}
        rowHeight={100}
        >
        {widgets.map((id, index) => (
          <div 
          key={id.toString()} 

          data-grid={{ 
            x: (index % 3) * 4,
            y: Math.floor(index / 3) * 4,
            w: 4,
            h: 4,
          }}

          className="relative group bg-slate-600 rounded-lg shadow-xl flex items-center justify-center text-white font-bold border border-white/5"
          >
          <div className="drag-handle absolute top-2 left-2 cursor-grab opacity-50 group-hover:opacity-100 p-1">
          ⠿
          </div>

          <button
          className="absolute top-2 right-2 text-xs p-1 bg-red-500 rounded opacity-0 group-hover:opacity-100 transition-opacity z-20"
          onMouseDown={(e) => e.stopPropagation()} 
          onClick={() => removeWidget(id)}
          >
          ✕
          </button>

          WIDGET {id}
          </div>

        ))}
        </Responsive>
      )}
      </div>
    </div>
  );
}

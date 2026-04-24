
export interface WidgetInterface {
  lowVal: number | null;
  highVal: number | null;
}

export interface WidgetListProps {
  items: WidgetInterface[];
}

export default function EndSimulation({items}: WidgetListProps) {
  console.log(items);
  function ParseWidget({lowVal, highVal} : WidgetInterface) {
    
    const lowString = lowVal !== null ? `lowVal: ${lowVal}` : "lowVal: not tracked";
    const highString = highVal !== null ? `highVal: ${highVal}` : "highVal: not tracked";
    return (
      <p>{lowString}   {highString}</p>
    );
  }
  const widgetList = items.map((item, index) => (
  <li key={index}> 
    <ParseWidget lowVal={item.lowVal} highVal={item.highVal}/> 
  </li>
));
  
  return (
    <div>
      <h1>result page</h1>
      <ul>
      {widgetList}
      </ul>
    </div>
  );
}

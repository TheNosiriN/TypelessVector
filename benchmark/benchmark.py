import pandas as pd
import plotly.graph_objects as go
from plotly.subplots import make_subplots



df = pd.read_json("resultsMSVC.json", orient='split');
dc = pd.read_json("resultsGCC.json", orient='split');
# print(dc);

fig = make_subplots(rows=1, cols=2, x_title="Number of Entries", y_title="Milliseconds (ms)", subplot_titles=["MSVC", "GCC"], figure = go.Figure(layout= {
    "title": "Standard Vector vs Typeless Vector vs TypesafeTypeless Vector (Performance)",
    "font": dict(
        family="Roboto Mono",
        size=18,
        color="Black"
    )
}));

fig.add_trace(go.Scatter(name = "Standard Vector", y = df["StandardVector"], x = df["columns"], fill='none', mode= 'lines+markers'), row=1, col=1);
fig.add_trace(go.Scatter(name = "Typeless Vector", y = df["TypelessVector"], x = df["columns"], fill='none', mode= 'lines+markers'), row=1, col=1);
fig.add_trace(go.Scatter(name = "TypesafeTypeless Vector", y = df["TypesafeTypelessVector"], x = df["columns"], fill='none', mode= 'lines+markers'), row=1, col=1);

fig.add_trace(go.Scatter(name = "Standard Vector", y = dc["StandardVector"], x = dc["columns"], fill='none', mode= 'lines+markers'), row=1, col=2);
fig.add_trace(go.Scatter(name = "Typeless Vector", y = dc["TypelessVector"], x = dc["columns"], fill='none', mode= 'lines+markers'), row=1, col=2);
fig.add_trace(go.Scatter(name = "TypesafeTypeless Vector", y = dc["TypesafeTypelessVector"], x = dc["columns"], fill='none', mode= 'lines+markers'), row=1, col=2);

fig.write_html('results.html', auto_open=True)

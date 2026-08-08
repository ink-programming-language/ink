// Translated from solution.cpp.

var MAXN = 100010;

var adj = cpp_array((10 * MAXN));

var dist = cpp_array((10 * MAXN));

var par = cpp_array((10 * MAXN));

func go1(idx: dynamic, l: dynamic, r: dynamic, ds: dynamic)
{
  if ((idx > 1))
  {
    var par = (idx / 2);
    adj[(par + ds)].push_back([0, (idx + ds)]);
  }
  if ((l == r))
  {
    adj[(idx + ds)].push_back([0, l]);
    return idx;
  }
  var m = (((l + r)) / 2);
  var ret = max(idx, max(go1((2 * idx), l, m, ds), go1(((2 * idx) + 1), (m + 1), r, ds)));
  return ret;
}

func go2(idx: dynamic, l: dynamic, r: dynamic, ds: dynamic)
{
  {
    var i = l;
    while ((i < ((r + 1))))
    {
      adj[i].push_back([0, (idx + ds)]);
      i += 1;
    }
  }
  if ((r > l))
  {
    var m = (((l + r)) / 2);
    go2((2 * idx), l, m, ds);
    go2(((2 * idx) + 1), (m + 1), r, ds);
  }
}

func go3(idx: dynamic, x: dynamic, y: dynamic, l: dynamic, r: dynamic, f: dynamic)
{
  if (((y < l) || (x > r)))
  {
    return;
  }
  if (((x >= l) && (y <= r)))
  {
    f(idx);
    return;
  }
  var m = (((x + y)) / 2);
  go3((2 * idx), x, m, l, r, f);
  go3(((2 * idx) + 1), (m + 1), y, l, r, f);
}

func dijsktra(src: dynamic)
{
  memset(dist, 0x3f3f3f3f, cpp_sizeof((dist)));
  dist[src] = 0;
  par[src] = src;
  var pq: dynamic;
  pq.push([dist[src], src]);
  while ((!pq.empty()))
  {
    var u = pq.top().second;
    var l = pq.top().first;
    pq.pop();
    if ((dist[u] == l))
    {
      for (var p in adj[u])
      {
        var ndist = (l + p.first);
        if ((ndist < dist[p.second]))
        {
          dist[p.second] = ndist;
          par[p.second] = u;
          pq.push([dist[p.second], p.second]);
        }
      }
    }
  }
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  var src: dynamic;
  scanf(" %d %d", (&(n)), (&(m)));
  scanf(" %d", (&(src)));
  src -= 1;
  var ds1 = ((n - 1) + 5);
  var ds2 = ((go1(1, 0, (n - 1), ds1) + ds1) + 5);
  go2(1, 0, (n - 1), ds2);
  while (cpp_update(m, "--"))
  {
    var tp: dynamic;
    scanf(" %d", (&(tp)));
    if ((tp == 1))
    {
      var u: dynamic;
      var v: dynamic;
      var w: dynamic;
      scanf(" %d %d", (&(u)), (&(v)));
      scanf(" %d", (&(w)));
      adj[(u - 1)].push_back([w, (v - 1)]);
    } else
    {
      var vtx: dynamic;
      var l: dynamic;
      var r: dynamic;
      var w: dynamic;
      scanf(" %d %d", (&(vtx)), (&(l)));
      scanf(" %d %d", (&(r)), (&(w)));
      l -= 1;
      r -= 1;
      vtx -= 1;
      go3(1, 0, (n - 1), l, r, __cpp_lambda_1);
    }
  }
  dijsktra(src);
  {
    var i = 0;
    while ((i < (n)))
    {
      printf("%lld%c", (if ((dist[i] == 0x3f3f3f3f3f3f3f3f)) -1 else dist[i]), " \n"[(i == (n - 1))]);
      i += 1;
    }
  }
  return 0;
}

func __cpp_lambda_1(idx: dynamic)
{
  if ((tp == 2))
  {
    adj[vtx].push_back([w, (idx + ds1)]);
  } else
  {
    adj[(idx + ds2)].push_back([w, vtx]);
  }
}

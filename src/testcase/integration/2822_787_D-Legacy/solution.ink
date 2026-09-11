// Translated from solution.cpp.

var MAXN: dynamic = 100010;

var adj: dynamic = cpp_array((10 * MAXN));

var dist: dynamic = cpp_array((10 * MAXN));

var par: dynamic = cpp_array((10 * MAXN));

func go1(idx: dynamic, l: dynamic, r: dynamic, ds: dynamic) -> dynamic
{
  if ((idx > 1))
  {
    var par: dynamic = (idx / 2);
    adj[(par + ds)].push_back([0, (idx + ds)]);
  }
  if ((l == r))
  {
    adj[(idx + ds)].push_back([0, l]);
    return idx;
  }
  var m: dynamic = (((l + r)) / 2);
  var ret: dynamic = max(idx, max(go1((2 * idx), l, m, ds), go1(((2 * idx) + 1), (m + 1), r, ds)));
  return ret;
}

func go2(idx: dynamic, l: dynamic, r: dynamic, ds: dynamic) -> dynamic
{
  {
    var i: dynamic = l;
    while ((i < ((r + 1))))
    {
      adj[i].push_back([0, (idx + ds)]);
      i += 1;
    }
  }
  if ((r > l))
  {
    var m: dynamic = (((l + r)) / 2);
    go2((2 * idx), l, m, ds);
    go2(((2 * idx) + 1), (m + 1), r, ds);
  }
}

func go3(idx: dynamic, x: dynamic, y: dynamic, l: dynamic, r: dynamic, f: dynamic) -> dynamic
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
  var m: dynamic = (((x + y)) / 2);
  go3((2 * idx), x, m, l, r, f);
  go3(((2 * idx) + 1), (m + 1), y, l, r, f);
}

func dijsktra(src: dynamic) -> dynamic
{
  memset(dist, 0x3f3f3f3f, cpp_sizeof((dist)));
  dist[src] = 0;
  par[src] = src;
  var pq: dynamic = cpp_uninitialized();
  pq.push([dist[src], src]);
  while ((!pq.empty()))
  {
    var u: dynamic = pq.top().second;
    var l: dynamic = pq.top().first;
    pq.pop();
    if ((dist[u] == l))
    {
      for (var p: dynamic in adj[u])
      {
        var ndist: dynamic = (l + p.first);
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

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var src: dynamic = cpp_uninitialized();
  scanf(" %d %d", (&(n)), (&(m)));
  scanf(" %d", (&(src)));
  src -= 1;
  var ds1: dynamic = ((n - 1) + 5);
  var ds2: dynamic = ((go1(1, 0, (n - 1), ds1) + ds1) + 5);
  go2(1, 0, (n - 1), ds2);
  while (cpp_update(m, "--"))
  {
    var tp: dynamic = cpp_uninitialized();
    scanf(" %d", (&(tp)));
    if ((tp == 1))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      var w: dynamic = cpp_uninitialized();
      scanf(" %d %d", (&(u)), (&(v)));
      scanf(" %d", (&(w)));
      adj[(u - 1)].push_back([w, (v - 1)]);
    } else
    {
      var vtx: dynamic = cpp_uninitialized();
      var l: dynamic = cpp_uninitialized();
      var r: dynamic = cpp_uninitialized();
      var w: dynamic = cpp_uninitialized();
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
    var i: dynamic = 0;
    while ((i < (n)))
    {
      printf("%lld%c", ( ((dist[i] == 0x3f3f3f3f3f3f3f3f)) ? -1 : dist[i]), " \n"[(i == (n - 1))]);
      i += 1;
    }
  }
  return 0;
}

func __cpp_lambda_1(idx: dynamic) -> dynamic
{
  if ((tp == 2))
  {
    adj[vtx].push_back([w, (idx + ds1)]);
  } else
  {
    adj[(idx + ds2)].push_back([w, vtx]);
  }
}

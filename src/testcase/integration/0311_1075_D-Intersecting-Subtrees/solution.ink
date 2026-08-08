// Translated from solution.cpp.

func enumerateSubmasks(m: dynamic)
{
  {
    var s = m;
    while (true)
    {
      if ((s == 0))
      {
        break;
      }
      s = (((s - 1)) & m);
    }
  }
}

func mpow(a: dynamic, b: dynamic, m: dynamic)
{
  if ((b == 0))
  {
    return 1;
  }
  var x = mpow(a, (b / 2), m);
  x = (((x * x)) % m);
  if ((b % 2))
  {
    x = (((x * a)) % m);
  }
  return x;
}

func update(s: dynamic, e: dynamic, qs: dynamic, qe: dynamic, seg: dynamic, lazy: dynamic, index: dynamic, value: dynamic)
{
  if ((lazy[index] != -1))
  {
    seg[index] = max(seg[index], lazy[index]);
    if ((s != e))
    {
      if ((lazy[(2 * index)] == -1))
      {
        lazy[(2 * index)] = lazy[index];
      } else
      {
        lazy[(2 * index)] = max(lazy[(2 * index)], lazy[index]);
      }
      if ((lazy[((2 * index) + 1)] == -1))
      {
        lazy[((2 * index) + 1)] = lazy[index];
      } else
      {
        lazy[((2 * index) + 1)] = max(lazy[((2 * index) + 1)], lazy[index]);
      }
    }
    lazy[index] = -1;
  }
  if (((qs > e) || (qe < s)))
  {
    return;
  }
  if (((s >= qs) && (e <= qe)))
  {
    seg[index] = max(seg[index], value);
    if ((s != e))
    {
      if ((lazy[(2 * index)] == -1))
      {
        lazy[(2 * index)] = value;
      } else
      {
        lazy[(2 * index)] = max(lazy[(2 * index)], value);
      }
      if ((lazy[((2 * index) + 1)] == -1))
      {
        lazy[((2 * index) + 1)] = value;
      } else
      {
        lazy[((2 * index) + 1)] = max(lazy[((2 * index) + 1)], value);
      }
    }
    return;
  }
  var mid = (((s + e)) / 2);
  update(s, mid, qs, qe, seg, lazy, (2 * index), value);
  update((mid + 1), e, qs, qe, seg, lazy, ((2 * index) + 1), value);
}

func query(s: dynamic, e: dynamic, qs: dynamic, qe: dynamic, seg: dynamic, lazy: dynamic, index: dynamic)
{
  if ((lazy[index] != -1))
  {
    seg[index] = max(seg[index], lazy[index]);
    if ((s != e))
    {
      if ((lazy[(2 * index)] == -1))
      {
        lazy[(2 * index)] = lazy[index];
      } else
      {
        lazy[(2 * index)] = max(lazy[(2 * index)], lazy[index]);
      }
      if ((lazy[((2 * index) + 1)] == -1))
      {
        lazy[((2 * index) + 1)] = lazy[index];
      } else
      {
        lazy[((2 * index) + 1)] = max(lazy[((2 * index) + 1)], lazy[index]);
      }
    }
    lazy[index] = -1;
  }
  if (((qs > e) || (qe < s)))
  {
    return LLONG_MIN;
  }
  if (((s >= qs) && (e <= qe)))
  {
    return seg[index];
  }
  var mid = (((s + e)) / 2);
  var a = query(s, mid, qs, qe, seg, lazy, (2 * index));
  var b = query((mid + 1), e, qs, qe, seg, lazy, ((2 * index) + 1));
  return max(a, b);
}

func printBinaryString(n: dynamic)
{
  var temp: dynamic;
  while (n)
  {
    if ((n & 1))
    {
      temp.push_back(1);
    } else
    {
      temp.push_back(0);
    }
    n = (n >> 1);
  }
  reverse(temp.begin(), temp.end());
  for (var node in temp)
  {
    write(node, " ");
  }
  write("\n");
}

func readVector(a: dynamic)
{
  var n = a.size();
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
}

class node
{
  var id: dynamic;
  var val: dynamic;
  var dir: dynamic;
}

var adj: dynamic;

var par: dynamic;

var x: dynamic;

var y: dynamic;

var k1: dynamic;

var k2: dynamic;

var answer: dynamic;

func interactA(x: dynamic)
{
  write("A ", x, "\n");
  var ret: dynamic;
  read(ret);
  fflush(stdout);
  return ret;
}

func interactB(x: dynamic)
{
  write("B ", x, "\n");
  var ret: dynamic;
  read(ret);
  fflush(stdout);
  return ret;
}

func solve(node: dynamic, par: dynamic, k: dynamic)
{
  var totalInSubtree = 1;
  for (var child in adj[node])
  {
    if ((child == par))
    {
      continue;
    }
    var ret = solve(child, node, k);
    var mila = ret.second;
    if (mila)
    {
      return [0, true];
    }
    totalInSubtree += ret.first;
  }
  if ((totalInSubtree < k))
  {
    return [totalInSubtree, false];
  }
  if ((x[node] == false))
  {
    return [0, false];
  }
  var bLabel = interactA(node);
  if (y[bLabel])
  {
    answer = node;
    return [0, true];
  } else
  {
    return [0, false];
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var tc: dynamic;
  read(tc);
  while (cpp_update(tc, "--"))
  {
    answer = -1;
    x.clear();
    y.clear();
    adj.clear();
    par.clear();
    var n: dynamic;
    read(n);
    {
      var i = 0;
      while ((i < (n - 1)))
      {
        var u: dynamic;
        var v: dynamic;
        read(u, v);
        adj[u].push_back(v);
        adj[v].push_back(u);
        i += 1;
      }
    }
    read(k1);
    {
      var i = 0;
      while ((i < k1))
      {
        var temp: dynamic;
        read(temp);
        x[temp] = true;
        i += 1;
      }
    }
    var st: dynamic;
    read(k2);
    {
      var i = 0;
      while ((i < k2))
      {
        var temp: dynamic;
        read(temp);
        st = temp;
        y[temp] = true;
        i += 1;
      }
    }
    var start = interactB(st);
    var bfs: dynamic;
    var toCompare: dynamic;
    bfs.push(start);
    var visited: dynamic;
    while ((!bfs.empty()))
    {
      var node = bfs.front();
      bfs.pop();
      visited[node] = true;
      if (x[node])
      {
        toCompare = node;
        break;
      }
      for (var child in adj[node])
      {
        if (visited[child])
        {
          continue;
        }
        bfs.push(child);
      }
    }
    var temp = interactA(toCompare);
    if (y[temp])
    {
      write("C ", toCompare, "\n");
    } else
    {
      write("C -1", "\n");
    }
    fflush(stdout);
  }
}

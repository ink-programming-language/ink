// Translated from solution.cpp.

func enumerateSubmasks(m: dynamic) -> dynamic
{
  {
    var s: dynamic = m;
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

func mpow(a: dynamic, b: dynamic, m: dynamic) -> dynamic
{
  if ((b == 0))
  {
    return 1;
  }
  var x: dynamic = mpow(a, (b / 2), m);
  x = (((x * x)) % m);
  if ((b % 2))
  {
    x = (((x * a)) % m);
  }
  return x;
}

func update(s: dynamic, e: dynamic, qs: dynamic, qe: dynamic, seg: dynamic, lazy: dynamic, index: dynamic, value: dynamic) -> dynamic
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
  var mid: dynamic = (((s + e)) / 2);
  update(s, mid, qs, qe, seg, lazy, (2 * index), value);
  update((mid + 1), e, qs, qe, seg, lazy, ((2 * index) + 1), value);
}

func query(s: dynamic, e: dynamic, qs: dynamic, qe: dynamic, seg: dynamic, lazy: dynamic, index: dynamic) -> dynamic
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
  var mid: dynamic = (((s + e)) / 2);
  var a: dynamic = query(s, mid, qs, qe, seg, lazy, (2 * index));
  var b: dynamic = query((mid + 1), e, qs, qe, seg, lazy, ((2 * index) + 1));
  return max(a, b);
}

func printBinaryString(n: dynamic) -> dynamic
{
  var temp: dynamic = cpp_uninitialized();
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
  for (var node: dynamic in temp)
  {
    write(node, " ");
  }
  write("\n");
}

func readVector(a: dynamic) -> dynamic
{
  var n: dynamic = a.size();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
}

class node
{
  var id: dynamic = cpp_uninitialized();
  var val: dynamic = cpp_uninitialized();
  var dir: dynamic = cpp_uninitialized();
}

var adj: dynamic = cpp_uninitialized();

var par: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var k1: dynamic = cpp_uninitialized();

var k2: dynamic = cpp_uninitialized();

var answer: dynamic = cpp_uninitialized();

func interactA(x: dynamic) -> dynamic
{
  write("A ", x, "\n");
  var ret: dynamic = cpp_uninitialized();
  read(ret);
  fflush(stdout);
  return ret;
}

func interactB(x: dynamic) -> dynamic
{
  write("B ", x, "\n");
  var ret: dynamic = cpp_uninitialized();
  read(ret);
  fflush(stdout);
  return ret;
}

func solve(node: dynamic, par: dynamic, k: dynamic) -> dynamic
{
  var totalInSubtree: dynamic = 1;
  for (var child: dynamic in adj[node])
  {
    if ((child == par))
    {
      continue;
    }
    var ret: dynamic = solve(child, node, k);
    var mila: dynamic = ret.second;
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
  var bLabel: dynamic = interactA(node);
  if (y[bLabel])
  {
    answer = node;
    return [0, true];
  } else
  {
    return [0, false];
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var tc: dynamic = cpp_uninitialized();
  read(tc);
  while (cpp_update(tc, "--"))
  {
    answer = -1;
    x.clear();
    y.clear();
    adj.clear();
    par.clear();
    var n: dynamic = cpp_uninitialized();
    read(n);
    {
      var i: dynamic = 0;
      while ((i < (n - 1)))
      {
        var u: dynamic = cpp_uninitialized();
        var v: dynamic = cpp_uninitialized();
        read(u, v);
        adj[u].push_back(v);
        adj[v].push_back(u);
        i += 1;
      }
    }
    read(k1);
    {
      var i: dynamic = 0;
      while ((i < k1))
      {
        var temp: dynamic = cpp_uninitialized();
        read(temp);
        x[temp] = true;
        i += 1;
      }
    }
    var st: dynamic = cpp_uninitialized();
    read(k2);
    {
      var i: dynamic = 0;
      while ((i < k2))
      {
        var temp: dynamic = cpp_uninitialized();
        read(temp);
        st = temp;
        y[temp] = true;
        i += 1;
      }
    }
    var start: dynamic = interactB(st);
    var bfs: dynamic = cpp_uninitialized();
    var toCompare: dynamic = cpp_uninitialized();
    bfs.push(start);
    var visited: dynamic = cpp_uninitialized();
    while ((!bfs.empty()))
    {
      var node: dynamic = bfs.front();
      bfs.pop();
      visited[node] = true;
      if (x[node])
      {
        toCompare = node;
        break;
      }
      for (var child: dynamic in adj[node])
      {
        if (visited[child])
        {
          continue;
        }
        bfs.push(child);
      }
    }
    var temp: dynamic = interactA(toCompare);
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

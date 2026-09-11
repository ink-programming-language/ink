// Translated from solution.cpp.

var N: dynamic = (5e5 + 10);

var components: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var in_cpp: dynamic = cpp_uninitialized();

var out: dynamic = cpp_uninitialized();

var start: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_array(N);

var p: dynamic = cpp_array(N);

var s: dynamic = cpp_array(N);

var need: dynamic = cpp_array((N + 1));

class Query
{
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var must_rollback: dynamic = cpp_uninitialized();
}

var tree: dynamic = cpp_array(((N + 1) << 2));

class Save
{
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var sizeU: dynamic = cpp_uninitialized();
  var sizeV: dynamic = cpp_uninitialized();
}

var ops: dynamic = cpp_uninitialized();

func find_set(x: dynamic) -> dynamic
{
  return  ((x == p[x])) ? x : find_set(p[x]);
}

func union_sets(x: dynamic, y: dynamic) -> dynamic
{
  x = find_set(x);
  y = find_set(y);
  if ((x == y))
  {
    return false;
  }
  if ((s[x] < s[y]))
  {
    swap(x, y);
  }
  p[y] = x;
  ops.push([x, y, s[x], s[y]]);
  s[x] += s[y];
  s[y] = s[x];
  return true;
}

func rollback() -> dynamic
{
  if (ops.empty())
  {
    return;
  }
  var op: dynamic = ops.top();
  ops.pop();
  p[op.u] = op.u;
  p[op.v] = op.v;
  s[op.u] = op.sizeU;
  s[op.v] = op.sizeV;
}

func add_query(l: dynamic, r: dynamic, L: dynamic, R: dynamic, q: dynamic, p: dynamic) -> dynamic
{
  if (((l > R) || (L > r)))
  {
    return;
  }
  if (((L <= l) && (R >= r)))
  {
    tree[p].push_back(q);
  } else
  {
    var m: dynamic = ((l + r) >> 1);
    add_query(l, m, L, R, q, (p << 1));
    add_query((m + 1), r, L, R, q, ((p << 1) | 1));
  }
}

func traverse(l: dynamic, r: dynamic, p: dynamic) -> dynamic
{
  for (var q: dynamic in tree[p])
  {
    q.must_rollback = union_sets(q.u, q.v);
  }
  if ((l == r))
  {
    for (var __cpp_item_1: dynamic in need[l])
    {
      var (x, index): dynamic = __cpp_item_1;
      ans[index] = s[find_set(x)];
    }
  } else
  {
    var m: dynamic = ((l + r) >> 1);
    traverse(l, m, (p << 1));
    traverse((m + 1), r, ((p << 1) | 1));
  }
  for (var q: dynamic in tree[p])
  {
    if (q.must_rollback)
    {
      rollback();
    }
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(n, m, k);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      p[i] = i;
      s[i] = 1;
      i += 1;
    }
  }
  var timer: dynamic = 1;
  var cnt: dynamic = 0;
  var days: dynamic = [0];
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var type_cpp: dynamic = cpp_uninitialized();
      read(type_cpp);
      if ((type_cpp == 1))
      {
        var p: dynamic = cpp_uninitialized();
        var q: dynamic = cpp_uninitialized();
        read(p, q);
        u.push_back(p);
        v.push_back(q);
        id.push_back(i);
        d.push_back((days.size() - 1));
      } else if ((type_cpp == 2))
      {
        var x: dynamic = cpp_uninitialized();
        read(x);
        need[i].push_back([x, cpp_update(cnt, "++")]);
      } else
      {
        days.push_back(i);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < v.size()))
    {
      var L: dynamic = id[i];
      var R: dynamic = N;
      var q: dynamic = [u[i], v[i], 0];
      if ((days.size() > (d[i] + k)))
      {
        R = (days[(d[i] + k)] - 1);
      }
      add_query(1, N, L, R, q, 1);
      i += 1;
    }
  }
  traverse(1, N, 1);
  {
    var i: dynamic = 1;
    while ((i <= cnt))
    {
      write(ans[i], cpp_char(" "));
      i += 1;
    }
  }
}

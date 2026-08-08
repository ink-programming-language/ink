// Translated from solution.cpp.

var N = (5e5 + 10);

var components: dynamic;

var n: dynamic;

var m: dynamic;

var k: dynamic;

var in_cpp: dynamic;

var out: dynamic;

var start: dynamic;

var ans = cpp_array(N);

var p = cpp_array(N);

var s = cpp_array(N);

var need = cpp_array((N + 1));

class Query
{
  var u: dynamic;
  var v: dynamic;
  var must_rollback: dynamic;
}

var tree = cpp_array(((N + 1) << 2));

class Save
{
  var u: dynamic;
  var v: dynamic;
  var sizeU: dynamic;
  var sizeV: dynamic;
}

var ops: dynamic;

func find_set(x: dynamic)
{
  return if ((x == p[x])) x else find_set(p[x]);
}

func union_sets(x: dynamic, y: dynamic)
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

func rollback()
{
  if (ops.empty())
  {
    return;
  }
  var op = ops.top();
  ops.pop();
  p[op.u] = op.u;
  p[op.v] = op.v;
  s[op.u] = op.sizeU;
  s[op.v] = op.sizeV;
}

func add_query(l: dynamic, r: dynamic, L: dynamic, R: dynamic, q: dynamic, p: dynamic)
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
    var m = ((l + r) >> 1);
    add_query(l, m, L, R, q, (p << 1));
    add_query((m + 1), r, L, R, q, ((p << 1) | 1));
  }
}

func traverse(l: dynamic, r: dynamic, p: dynamic)
{
  for (var q in tree[p])
  {
    q.must_rollback = union_sets(q.u, q.v);
  }
  if ((l == r))
  {
    for (var __cpp_item_1 in need[l])
    {
      var (x, index) = __cpp_item_1;
      ans[index] = s[find_set(x)];
    }
  } else
  {
    var m = ((l + r) >> 1);
    traverse(l, m, (p << 1));
    traverse((m + 1), r, ((p << 1) | 1));
  }
  for (var q in tree[p])
  {
    if (q.must_rollback)
    {
      rollback();
    }
  }
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(n, m, k);
  {
    var i = 1;
    while ((i <= n))
    {
      p[i] = i;
      s[i] = 1;
      i += 1;
    }
  }
  var timer = 1;
  var cnt = 0;
  var days = [0];
  var u: dynamic;
  var v: dynamic;
  var id: dynamic;
  var d: dynamic;
  {
    var i = 1;
    while ((i <= m))
    {
      var type_cpp: dynamic;
      read(type_cpp);
      if ((type_cpp == 1))
      {
        var p: dynamic;
        var q: dynamic;
        read(p, q);
        u.push_back(p);
        v.push_back(q);
        id.push_back(i);
        d.push_back((days.size() - 1));
      } else if ((type_cpp == 2))
      {
        var x: dynamic;
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
    var i = 0;
    while ((i < v.size()))
    {
      var L = id[i];
      var R = N;
      var q = [u[i], v[i], 0];
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
    var i = 1;
    while ((i <= cnt))
    {
      write(ans[i], cpp_char(" "));
      i += 1;
    }
  }
}

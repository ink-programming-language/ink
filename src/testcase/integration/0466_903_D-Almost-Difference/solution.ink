// Translated from solution.cpp.

var maxn: dynamic = (6e5 + 100);

var mx: dynamic = (1e9 + 10);

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(maxn);

var sum: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var cc: dynamic = cpp_array(maxn);

var tree: dynamic = cpp_array(maxn);

var v: dynamic = cpp_uninitialized();

func getid(x: dynamic) -> dynamic
{
  return ((lower_bound(v.begin(), v.end(), x) - v.begin()) + 1);
}

func __cpp_top_level_1() -> dynamic
{
}

func lowbit(x: dynamic) -> dynamic
{
  return (x & ((-x)));
}

func add(id: dynamic, x: dynamic) -> dynamic
{
  {
    var i: dynamic = id;
    while ((i <= maxn))
    {
      tree[i] += x;
      i += lowbit(i);
    }
  }
}

func getsum(x: dynamic) -> dynamic
{
  var ret: dynamic = 0;
  {
    var i: dynamic = x;
    while (i)
    {
      ret += tree[i];
      i -= lowbit(i);
    }
  }
  return ret;
}

func adds(id: dynamic) -> dynamic
{
  {
    var i: dynamic = id;
    while ((i <= maxn))
    {
      cc[i] += 1;
      i += lowbit(i);
    }
  }
}

func getsums(x: dynamic) -> dynamic
{
  var ret: dynamic = 0;
  {
    var i: dynamic = x;
    while (i)
    {
      ret += cc[i];
      i -= lowbit(i);
    }
  }
  return ret;
}

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      v.push_back(a[i]);
      v.push_back((a[i] + 1));
      v.push_back((a[i] - 2));
      i += 1;
    }
  }
  sort(v.begin(), v.end());
  v.erase(unique(v.begin(), v.end()), v.end());
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var id: dynamic = getid(a[i]);
      var id1: dynamic = getid((a[i] + 1));
      var id2: dynamic = getid((a[i] - 2));
      ans += ((a[i] * (((i - 1) - ((getsums(id1) - getsums(id2)))))) - ((sum - ((getsum(id1) - getsum(id2))))));
      sum += a[i];
      add(id, a[i]);
      adds(id);
      i += 1;
    }
  }
  write(setprecision(0), fixed);
  write(ans, "\n");
  return 0;
}

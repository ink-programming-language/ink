// Translated from solution.cpp.

var maxn = (6e5 + 100);

var mx = (1e9 + 10);

var n: dynamic;

var a = cpp_array(maxn);

var sum: dynamic;

var ans: dynamic;

var cc = cpp_array(maxn);

var tree = cpp_array(maxn);

var v: dynamic;

func getid(x: dynamic)
{
  return ((lower_bound(v.begin(), v.end(), x) - v.begin()) + 1);
}

func __cpp_top_level_1()
{
}

func lowbit(x: dynamic)
{
  return (x & ((-x)));
}

func add(id: dynamic, x: dynamic)
{
  {
    var i = id;
    while ((i <= maxn))
    {
      tree[i] += x;
      i += lowbit(i);
    }
  }
}

func getsum(x: dynamic)
{
  var ret = 0;
  {
    var i = x;
    while (i)
    {
      ret += tree[i];
      i -= lowbit(i);
    }
  }
  return ret;
}

func adds(id: dynamic)
{
  {
    var i = id;
    while ((i <= maxn))
    {
      cc[i] += 1;
      i += lowbit(i);
    }
  }
}

func getsums(x: dynamic)
{
  var ret = 0;
  {
    var i = x;
    while (i)
    {
      ret += cc[i];
      i -= lowbit(i);
    }
  }
  return ret;
}

func main()
{
  read(n);
  {
    var i = 1;
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
    var i = 1;
    while ((i <= n))
    {
      var id = getid(a[i]);
      var id1 = getid((a[i] + 1));
      var id2 = getid((a[i] - 2));
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

// Translated from solution.cpp.

var N: dynamic = (1e6 + 100);

var tree: dynamic = cpp_array(N);

var a: dynamic = cpp_array(N);

var sz: dynamic = cpp_array(N);

var mapping: dynamic = cpp_array(N);

var par: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

func print() -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((a[i] == false))
      {
        printf("%d ", i);
      }
      i += 1;
    }
  }
}

func query(idx: dynamic) -> dynamic
{
  var sum: dynamic = 0;
  while ((idx > 0))
  {
    sum += tree[idx];
    idx -= (idx & ((-idx)));
  }
  return sum;
}

func print(n: dynamic) -> dynamic
{
  write("--------------------\n");
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      write(i, " ", query(mapping[i]), " ", sz[i], " mapp ", mapping[i], "\n");
      i += 1;
    }
  }
  write("-----------------\n");
}

func update(idx: dynamic, x: dynamic, n: dynamic) -> dynamic
{
  while ((idx <= n))
  {
    tree[idx] += x;
    idx += (idx & ((-idx)));
  }
}

func rangeUpdate(x: dynamic, y: dynamic, val: dynamic, n: dynamic) -> dynamic
{
  update(x, val, n);
  update((y + 1), (-val), n);
}

var cc: dynamic = 1;

var len: dynamic = cpp_uninitialized();

var v: dynamic = cpp_array(N);

func dfs(x: dynamic, pr: dynamic, dis: dynamic) -> dynamic
{
  len += 1;
  par[x] = pr;
  mapping[x] = cc;
  rangeUpdate(cc, cc, dis, n);
  cc += 1;
  var st: dynamic = len;
  {
    var i: dynamic = 0;
    while ((i < v[x].size()))
    {
      var y: dynamic = v[x][i];
      if ((y != pr))
      {
        dfs(y, x, (dis + 1));
      }
      i += 1;
    }
  }
  sz[x] = (len - st);
}

func main() -> dynamic
{
  var q: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = 0;
  var temp: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  var sum: dynamic = 0;
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
  var cnt: dynamic = 0;
  var m: dynamic = cpp_uninitialized();
  var fg: dynamic = 0;
  var mx: dynamic = 0;
  var mx1: dynamic = 0;
  var mn: dynamic = 8000000000000000000;
  var mn1: dynamic = 8000000000000000000;
  scanf("%lld %lld", (&n), (&k));
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      scanf("%lld %lld", (&x), (&y));
      v[x].push_back(y);
      v[y].push_back(x);
      i += 1;
    }
  }
  dfs(n, n, 0);
  if ((k == 0))
  {
    print();
    return 0;
  }
  a[n] = true;
  k = (n - k);
  k -= 1;
  {
    var i: dynamic = (n - 1);
    while ((i >= 1))
    {
      if ((!k))
      {
        break;
      }
      if (a[i])
      {
        i -= 1;
        continue;
      }
      var val: dynamic = query(mapping[i]);
      if ((val > k))
      {
        i -= 1;
        continue;
      }
      k -= val;
      j = i;
      while ((!a[j]))
      {
        a[j] = true;
        rangeUpdate(mapping[j], (mapping[j] + sz[j]), -1, n);
        j = par[j];
      }
      i -= 1;
    }
  }
  print();
}

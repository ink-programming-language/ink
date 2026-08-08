// Translated from solution.cpp.

var N = (1e6 + 100);

var tree = cpp_array(N);

var a = cpp_array(N);

var sz = cpp_array(N);

var mapping = cpp_array(N);

var par = cpp_array(N);

var n: dynamic;

func print()
{
  {
    var i = 1;
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

func query(idx: dynamic)
{
  var sum = 0;
  while ((idx > 0))
  {
    sum += tree[idx];
    idx -= (idx & ((-idx)));
  }
  return sum;
}

func print(n: dynamic)
{
  write("--------------------\n");
  {
    var i = 1;
    while ((i <= n))
    {
      write(i, " ", query(mapping[i]), " ", sz[i], " mapp ", mapping[i], "\n");
      i += 1;
    }
  }
  write("-----------------\n");
}

func update(idx: dynamic, x: dynamic, n: dynamic)
{
  while ((idx <= n))
  {
    tree[idx] += x;
    idx += (idx & ((-idx)));
  }
}

func rangeUpdate(x: dynamic, y: dynamic, val: dynamic, n: dynamic)
{
  update(x, val, n);
  update((y + 1), (-val), n);
}

var cc = 1;

var len: dynamic;

var v = cpp_array(N);

func dfs(x: dynamic, pr: dynamic, dis: dynamic)
{
  len += 1;
  par[x] = pr;
  mapping[x] = cc;
  rangeUpdate(cc, cc, dis, n);
  cc += 1;
  var st = len;
  {
    var i = 0;
    while ((i < v[x].size()))
    {
      var y = v[x][i];
      if ((y != pr))
      {
        dfs(y, x, (dis + 1));
      }
      i += 1;
    }
  }
  sz[x] = (len - st);
}

func main()
{
  var q: dynamic;
  var i: dynamic;
  var j = 0;
  var temp: dynamic;
  var t: dynamic;
  var k: dynamic;
  var ans = 0;
  var sum = 0;
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
  var cnt = 0;
  var m: dynamic;
  var fg = 0;
  var mx = 0;
  var mx1 = 0;
  var mn = 8000000000000000000;
  var mn1 = 8000000000000000000;
  scanf("%lld %lld", (&n), (&k));
  {
    var i = 0;
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
    var i = (n - 1);
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
      var val = query(mapping[i]);
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

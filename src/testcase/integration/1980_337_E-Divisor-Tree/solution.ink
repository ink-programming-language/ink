// Translated from solution.cpp.

var n: dynamic;

var a = cpp_array(8);

var factors = cpp_array(8);

var primeCnt: dynamic;

func solve(cur: dynamic, tree: dynamic, rootNum: dynamic, rootSum: dynamic)
{
  if ((cur == n))
  {
    return (((n + ((rootNum > 1))) + rootSum) - primeCnt);
  }
  var ret = (1 << 28);
  {
    var i = 0;
    while ((i < cpp_cast((tree.size()))))
    {
      if (((tree[i] % a[cur]) == 0))
      {
        tree.push_back(a[cur]);
        tree[i] /= a[cur];
        ret = min(ret, solve((cur + 1), tree, (rootNum + ((i == 0))), (rootSum + (((i == 0)) * factors[cur]))));
        tree[i] *= a[cur];
        tree.pop_back();
      }
      i += 1;
    }
  }
  return ret;
}

func main()
{
  ios_base.sync_with_stdio(0);
  read(n);
  {
    var i = 0;
    while ((i < cpp_cast((n))))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a, (a + n), greater());
  {
    var i = 0;
    while ((i < cpp_cast((n))))
    {
      var tmp = a[i];
      {
        var j = 2;
        while (((cpp_cast(j) * j) <= tmp))
        {
          while (((tmp % j) == 0))
          {
            factors[i] += 1;
            tmp /= j;
          }
          j += 1;
        }
      }
      if ((tmp > 1))
      {
        factors[i] += 1;
      }
      if ((tmp == a[i]))
      {
        primeCnt += 1;
      }
      i += 1;
    }
  }
  var tree: dynamic;
  tree.push_back(0);
  var ret = solve(0, tree, 0, 0);
  write(ret, "\n");
  return 0;
}

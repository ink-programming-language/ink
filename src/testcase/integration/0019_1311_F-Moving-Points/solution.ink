// Translated from solution.cpp.

class FenwickTree
{
  var n: dynamic;
  var num: dynamic;
  func FenwickTree()
  {
      this->n = cpp_construct(0);
    }
  func FenwickTree(n: dynamic)
  {
      n = n;
      num.assign(n, 0);
    }
  func add(i: dynamic, val: dynamic)
  {
      {
        while ((i < n))
        {
          num[i] += val;
          i |= (i + 1);
        }
      }
    }
  func sum(i: dynamic)
  {
      var ret = 0;
      {
        while ((i >= 0))
        {
          ret += num[i];
          i = (((i & ((i + 1)))) - 1);
        }
      }
      return ret;
    }
}

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  var speed: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&point[i].first));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&point[i].second));
      speed.push_back(point[i].second);
      i += 1;
    }
  }
  sort(speed.begin(), speed.end());
  speed.erase(unique(speed.begin(), speed.end()), speed.end());
  sort(point.begin(), point.end());
  var ans = 0;
  for (var i in point)
  {
    i.second = (lower_bound(speed.begin(), speed.end(), i.second) - speed.begin());
    ans += ((cnt.sum(i.second) * i.first) - sumx.sum(i.second));
    cnt.add(i.second, 1);
    sumx.add(i.second, i.first);
  }
  printf("%lld\n", ans);
  return 0;
}

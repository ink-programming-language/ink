// Translated from solution.cpp.

class FenwickTree
{
  var n: dynamic = cpp_uninitialized();
  var num: dynamic = cpp_uninitialized();
  func FenwickTree() -> dynamic
  {
      self->n = cpp_construct(0);
    }
  func FenwickTree(n: dynamic) -> dynamic
  {
      n = n;
      num.assign(n, 0);
    }
  func add(i: dynamic, val: dynamic) -> dynamic
  {
      {
        while ((i < n))
        {
          num[i] += val;
          i |= (i + 1);
        }
      }
    }
  func sum(i: dynamic) -> dynamic
  {
      var ret: dynamic = 0;
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

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  var speed: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (&point[i].first));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
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
  var ans: dynamic = 0;
  for (var i: dynamic in point)
  {
    i.second = (lower_bound(speed.begin(), speed.end(), i.second) - speed.begin());
    ans += ((cnt.sum(i.second) * i.first) - sumx.sum(i.second));
    cnt.add(i.second, 1);
    sumx.add(i.second, i.first);
  }
  printf("%lld\n", ans);
  return 0;
}

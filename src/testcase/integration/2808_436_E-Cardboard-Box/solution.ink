// Translated from solution.cpp.

class Level
{
  var one: dynamic = cpp_uninitialized();
  var two: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
  func getOneKey() -> dynamic
  {
      return make_pair(one, (2 * id));
    }
  func getTwoKey() -> dynamic
  {
      return make_pair((two - one), ((2 * id) + 1));
    }
  func operator_less(a: dynamic) -> dynamic
  {
      return (two < a.two);
    }
}

class FenwickTree
{
  var keys: dynamic = cpp_uninitialized();
  var sum: dynamic = cpp_uninitialized();
  var cnt: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  func update(x: dynamic, delta: dynamic) -> dynamic
  {
      {
        var i: dynamic = x;
        while ((i < n))
        {
          sum[i] += (delta * keys[x].first);
          cnt[i] += delta;
          i |= (i + 1);
        }
      }
    }
  func get(x: dynamic) -> dynamic
  {
      var res: dynamic = cpp_uninitialized();
      {
        var i: dynamic = x;
        while ((i >= 0))
        {
          res.first += sum[i];
          res.second += cnt[i];
          i -= ((~i) & ((i + 1)));
        }
      }
      return res;
    }
  func FenwickTree(key: dynamic) -> dynamic
  {
      sort(key.begin(), key.end());
      n = key.size();
      sum.assign(n, 0);
      cnt.assign(n, 0);
      keys = key;
    }
  func update(key: dynamic, delta: dynamic) -> dynamic
  {
      var pos: dynamic = (lower_bound(keys.begin(), keys.end(), key) - keys.begin());
      assert((keys.at(pos) == key));
      update(pos, delta);
    }
  func minElementSum(m: dynamic) -> dynamic
  {
      if ((m <= 0))
      {
        return 0;
      }
      if ((m > get((n - 1)).second))
      {
        return cpp_cast(1e18);
      }
      var low: dynamic = 0;
      var high: dynamic = (n - 1);
      while ((low < high))
      {
        var mid: dynamic = (((low + high)) / 2);
        if ((get(mid).second >= m))
        {
          high = mid;
        } else
        {
          low = (mid + 1);
        }
      }
      return get(low).first;
    }
}

var N: dynamic = cpp_cast(3e5);

var INF: dynamic = cpp_cast(2e9);

var level: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  read(n, w);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(level[i].one, level[i].two);
      level[i].id = i;
      i += 1;
    }
  }
  sort(level, (level + n));
  var value: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      value.push_back(level[i].getOneKey());
      value.push_back(level[i].getTwoKey());
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      tree.update(level[i].getOneKey(), 1);
      i += 1;
    }
  }
  var bestCost: dynamic = cpp_cast(1e18);
  var firstCost: dynamic = 0;
  var bestPrefix: dynamic = -1;
  {
    var prefix: dynamic = 0;
    while ((prefix <= n))
    {
      var secondCost: dynamic = tree.minElementSum((w - prefix));
      if (((firstCost + secondCost) < bestCost))
      {
        bestCost = (firstCost + secondCost);
        bestPrefix = prefix;
      }
      if ((prefix < n))
      {
        tree.update(level[prefix].getOneKey(), -1);
        tree.update(level[prefix].getTwoKey(), 1);
        firstCost += level[prefix].one;
      }
      prefix += 1;
    }
  }
  var res: dynamic = cpp_construct(n, cpp_char("0"));
  {
    var i: dynamic = 0;
    while ((i < bestPrefix))
    {
      res[level[i].id] = cpp_char("1");
      i += 1;
    }
  }
  var q: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < bestPrefix))
    {
      q.push(level[i].getTwoKey());
      i += 1;
    }
  }
  {
    var i: dynamic = bestPrefix;
    while ((i < n))
    {
      q.push(level[i].getOneKey());
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (w - bestPrefix)))
    {
      res[(q.top().second / 2)] += 1;
      q.pop();
      i += 1;
    }
  }
  write(bestCost, cpp_char("\n"));
  write(res, cpp_char("\n"));
  return 0;
}

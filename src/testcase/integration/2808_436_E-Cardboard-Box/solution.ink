// Translated from solution.cpp.

class Level
{
  var one: dynamic;
  var two: dynamic;
  var id: dynamic;
  func getOneKey()
  {
      return make_pair(one, (2 * id));
    }
  func getTwoKey()
  {
      return make_pair((two - one), ((2 * id) + 1));
    }
  func operator_less(a: dynamic)
  {
      return (two < a.two);
    }
}

class FenwickTree
{
  var keys: dynamic;
  var sum: dynamic;
  var cnt: dynamic;
  var n: dynamic;
  func update(x: dynamic, delta: dynamic)
  {
      {
        var i = x;
        while ((i < n))
        {
          sum[i] += (delta * keys[x].first);
          cnt[i] += delta;
          i |= (i + 1);
        }
      }
    }
  func get(x: dynamic)
  {
      var res: dynamic;
      {
        var i = x;
        while ((i >= 0))
        {
          res.first += sum[i];
          res.second += cnt[i];
          i -= ((~i) & ((i + 1)));
        }
      }
      return res;
    }
  func FenwickTree(key: dynamic)
  {
      sort(key.begin(), key.end());
      n = key.size();
      sum.assign(n, 0);
      cnt.assign(n, 0);
      keys = key;
    }
  func update(key: dynamic, delta: dynamic)
  {
      var pos = (lower_bound(keys.begin(), keys.end(), key) - keys.begin());
      assert((keys.at(pos) == key));
      update(pos, delta);
    }
  func minElementSum(m: dynamic)
  {
      if ((m <= 0))
      {
        return 0;
      }
      if ((m > get((n - 1)).second))
      {
        return cpp_cast(1e18);
      }
      var low = 0;
      var high = (n - 1);
      while ((low < high))
      {
        var mid = (((low + high)) / 2);
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

var N = cpp_cast(3e5);

var INF = cpp_cast(2e9);

var level = cpp_array(N);

var n: dynamic;

var w: dynamic;

func main()
{
  ios.sync_with_stdio(false);
  read(n, w);
  {
    var i = 0;
    while ((i < n))
    {
      read(level[i].one, level[i].two);
      level[i].id = i;
      i += 1;
    }
  }
  sort(level, (level + n));
  var value: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      value.push_back(level[i].getOneKey());
      value.push_back(level[i].getTwoKey());
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      tree.update(level[i].getOneKey(), 1);
      i += 1;
    }
  }
  var bestCost = cpp_cast(1e18);
  var firstCost = 0;
  var bestPrefix = -1;
  {
    var prefix = 0;
    while ((prefix <= n))
    {
      var secondCost = tree.minElementSum((w - prefix));
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
  var res = cpp_construct(n, cpp_char("0"));
  {
    var i = 0;
    while ((i < bestPrefix))
    {
      res[level[i].id] = cpp_char("1");
      i += 1;
    }
  }
  var q: dynamic;
  {
    var i = 0;
    while ((i < bestPrefix))
    {
      q.push(level[i].getTwoKey());
      i += 1;
    }
  }
  {
    var i = bestPrefix;
    while ((i < n))
    {
      q.push(level[i].getOneKey());
      i += 1;
    }
  }
  {
    var i = 0;
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

// Translated from solution.cpp.

var MAXN = (1e5 + 20);

var n: dynamic;

var k: dynamic;

var M: dynamic;

var inv = cpp_array(MAXN);

var pre_inv = cpp_array(MAXN);

func math_pre()
{
  inv[1] = 1;
  {
    var i = 2;
    while ((i <= (if (((n < 4))) 4 else n)))
    {
      inv[i] = (((1 * ((M - (M / i)))) * inv[(M % i)]) % M);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      pre_inv[i] = (((pre_inv[(i - 1)] + inv[i])) % M);
      i += 1;
    }
  }
}

class map
{
  var MAXMap: dynamic;
  var tot: dynamic;
  var node: dynamic = cpp_array((MAXMap + 1));
  func map()
  {
      tot = 0;
    }
  func find(key: dynamic)
  {
      var ret = node;
      while ((((ret - node) < tot) && (ret->key != key)))
      {
        ret += 1;
      }
      return ret;
    }
  func insert(new_element: dynamic)
  {
      node[cpp_update(tot, "++")] = new_element;
    }
  func begin()
  {
      return (&node[0]);
    }
  func end()
  {
      return (&node[tot]);
    }
}

var Map: dynamic;

func solve(l: dynamic, r: dynamic, h: dynamic)
{
  if (((l >= r) || (h <= 1)))
  {
    var len = ((r - l) + 1);
    var it = Map.find(len);
    if ((it == Map.end()))
    {
      Map.insert(map.pad(len, 1));
    } else
    {
      it->val += 1;
    }
    return;
  }
  var mid = (((l + r)) >> 1);
  solve(l, mid, (h - 1));
  solve((mid + 1), r, (h - 1));
}

func calc(len1: dynamic, len2: dynamic)
{
  var ret = 0;
  {
    var i = 1;
    while ((i <= len1))
    {
      ret = (((((((ret + (((1 * inv[2]) * len2) % M)) - ((pre_inv[(i + len2)] - pre_inv[((i + 1) - 1)])))) % M) + M)) % M);
      i += 1;
    }
  }
  return ret;
}

func main()
{
  scanf("%d%d%d", (&n), (&k), (&M));
  math_pre();
  solve(1, n, k);
  var ans = 0;
  {
    var it = Map.begin();
    while ((it != Map.end()))
    {
      var len = it->key;
      var cnt = it->val;
      ans = (((ans + (((((((1 * cnt) * len) % M) * ((len - 1))) % M) * inv[4]) % M))) % M);
      it += 1;
    }
  }
  {
    var it1 = Map.begin();
    while ((it1 != Map.end()))
    {
      {
        var it2 = Map.begin();
        while ((it2 != Map.end()))
        {
          if ((it1 == it2))
          {
            var len = it1->key;
            var cnt = ((((1 * ((0 + ((it1->val - 1))))) * it1->val) / 2) % M);
            ans = (((ans + (((1 * cnt) * calc(len, len)) % M))) % M);
          } else if ((it1->key < it2->key))
          {
            var len1 = it1->key;
            var len2 = it2->key;
            var cnt = (((1 * it1->val) * it2->val) % M);
            ans = (((ans + (((1 * cnt) * calc(len1, len2)) % M))) % M);
          }
          it2 += 1;
        }
      }
      it1 += 1;
    }
  }
  printf("%d", ans);
}

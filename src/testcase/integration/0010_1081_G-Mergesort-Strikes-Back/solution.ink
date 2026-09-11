// Translated from solution.cpp.

var MAXN: dynamic = (1e5 + 20);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

var inv: dynamic = cpp_array(MAXN);

var pre_inv: dynamic = cpp_array(MAXN);

func math_pre() -> dynamic
{
  inv[1] = 1;
  {
    var i: dynamic = 2;
    while ((i <= ( (((n < 4))) ? 4 : n)))
    {
      inv[i] = (((1 * ((M - (M / i)))) * inv[(M % i)]) % M);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      pre_inv[i] = (((pre_inv[(i - 1)] + inv[i])) % M);
      i += 1;
    }
  }
}

class map
{
  var MAXMap: dynamic = cpp_uninitialized();
  var tot: dynamic = cpp_uninitialized();
  var node: dynamic = cpp_array((MAXMap + 1));
  func map() -> dynamic
  {
      tot = 0;
    }
  func find(key: dynamic) -> dynamic
  {
      var ret: dynamic = node;
      while ((((ret - node) < tot) && (ret->key != key)))
      {
        ret += 1;
      }
      return ret;
    }
  func insert(new_element: dynamic) -> dynamic
  {
      node[cpp_update(tot, "++")] = new_element;
    }
  func begin() -> dynamic
  {
      return (&node[0]);
    }
  func end() -> dynamic
  {
      return (&node[tot]);
    }
}

var Map: dynamic = cpp_uninitialized();

func solve(l: dynamic, r: dynamic, h: dynamic) -> dynamic
{
  if (((l >= r) || (h <= 1)))
  {
    var len: dynamic = ((r - l) + 1);
    var it: dynamic = Map.find(len);
    if ((it == Map.end()))
    {
      Map.insert(map.pad(len, 1));
    } else
    {
      it->val += 1;
    }
    return;
  }
  var mid: dynamic = (((l + r)) >> 1);
  solve(l, mid, (h - 1));
  solve((mid + 1), r, (h - 1));
}

func calc(len1: dynamic, len2: dynamic) -> dynamic
{
  var ret: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= len1))
    {
      ret = (((((((ret + (((1 * inv[2]) * len2) % M)) - ((pre_inv[(i + len2)] - pre_inv[((i + 1) - 1)])))) % M) + M)) % M);
      i += 1;
    }
  }
  return ret;
}

func main() -> dynamic
{
  scanf("%d%d%d", (&n), (&k), (&M));
  math_pre();
  solve(1, n, k);
  var ans: dynamic = 0;
  {
    var it: dynamic = Map.begin();
    while ((it != Map.end()))
    {
      var len: dynamic = it->key;
      var cnt: dynamic = it->val;
      ans = (((ans + (((((((1 * cnt) * len) % M) * ((len - 1))) % M) * inv[4]) % M))) % M);
      it += 1;
    }
  }
  {
    var it1: dynamic = Map.begin();
    while ((it1 != Map.end()))
    {
      {
        var it2: dynamic = Map.begin();
        while ((it2 != Map.end()))
        {
          if ((it1 == it2))
          {
            var len: dynamic = it1->key;
            var cnt: dynamic = ((((1 * ((0 + ((it1->val - 1))))) * it1->val) / 2) % M);
            ans = (((ans + (((1 * cnt) * calc(len, len)) % M))) % M);
          } else if ((it1->key < it2->key))
          {
            var len1: dynamic = it1->key;
            var len2: dynamic = it2->key;
            var cnt: dynamic = (((1 * it1->val) * it2->val) % M);
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

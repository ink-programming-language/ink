// Translated from solution.cpp.

class outputer
{
}

class outputable
{
}

func sqr(x: dynamic) -> dynamic
{
  return (x * x);
}

func umx(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func umn(a: dynamic, b: dynamic) -> dynamic
{
  if ((b < a))
  {
    a = b;
    return 1;
  }
  return 0;
}

var N: dynamic = 100000;

var M: dynamic = 30;

class Input
{
  var n: dynamic = cpp_uninitialized();
  var st: dynamic = cpp_array(N);
  var fn: dynamic = cpp_array(N);
  func read() -> dynamic
  {
      if ((!((cin >> n))))
      {
        return 0;
      }
      {
        var i: dynamic = int_cpp(0);
        while ((i < int_cpp(n)))
        {
          scanf("%u%u%u%u", (&st[i].first), (&st[i].second), (&fn[i].first), (&fn[i].second));
          i += 1;
        }
      }
      return 1;
    }
  func init(input: dynamic) -> dynamic
  {
      (*self) = input;
    }
}

class Data
{
  var ans: dynamic = cpp_uninitialized();
  func write() -> dynamic
  {
      write(ans, "\n");
    }
}

var K: dynamic = (4 * N);

func get_last_one(x: dynamic) -> dynamic
{
  var res: dynamic = -1;
  {
    var i: dynamic = (int_cpp(5) - 1);
    while ((i >= int_cpp(0)))
    {
      if ((x & (-((1 << ((res + ((1 << i)))))))))
      {
        res += (1 << i);
      }
      i -= 1;
    }
  }
  return res;
}

func cut(a: dynamic, d: dynamic) -> dynamic
{
  return pair((a.first & (-((1 << ((d + 1)))))), (a.second & (-((1 << ((d + 1)))))));
}

func common(a: dynamic, b: dynamic) -> dynamic
{
  var d: dynamic = max(get_last_one((a.first ^ b.first)), get_last_one((a.second ^ b.second)));
  if ((d == -1))
  {
    return a;
  }
  if ((((a.first & ((1 << d)))) && ((b.second & ((1 << d))))))
  {
    return cut(a, d);
  }
  if ((((a.second & ((1 << d)))) && ((b.first & ((1 << d))))))
  {
    return cut(a, d);
  }
  if ((a.first & ((1 << d))))
  {
    return cut(b, get_last_one((b.second & ((((1 << d)) - 1)))));
  }
  if ((a.second & ((1 << d))))
  {
    return cut(b, get_last_one((b.first & ((((1 << d)) - 1)))));
  }
  if ((b.first & ((1 << d))))
  {
    return cut(a, get_last_one((a.second & ((((1 << d)) - 1)))));
  }
  if ((b.second & ((1 << d))))
  {
    return cut(a, get_last_one((a.first & ((((1 << d)) - 1)))));
  }
  assert(0);
}

func ord(a: dynamic) -> dynamic
{
  var res: dynamic = 0;
  {
    var i: dynamic = (int_cpp(30) - 1);
    while ((i >= int_cpp(0)))
    {
      if ((!a.second))
      {
        return (res + a.first);
      }
      if ((a.first & ((1 << i))))
      {
        a.first -= (1 << i);
        res += (1 << i);
        i -= 1;
        continue;
      }
      if ((a.second & ((1 << i))))
      {
        a.second -= (1 << i);
        res += (1 << (((i + i) + 1)));
        i -= 1;
        continue;
      }
      res += (1 << ((i + i)));
      i -= 1;
    }
  }
  return res;
}

class Solution
{
  var v_cnt: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_array(K);
  var num: dynamic = cpp_uninitialized();
  var pr: dynamic = cpp_array(K);
  var lvl: dynamic = cpp_array(K);
  var inc: dynamic = cpp_array(K);
  var dec: dynamic = cpp_array(K);
  var tmp: dynamic = cpp_array(K);
  func sort_unique() -> dynamic
  {
      {
        var i: dynamic = int_cpp(0);
        while ((i < int_cpp(v_cnt)))
        {
          tmp[i] = make_pair(ord(a[i]), a[i]);
          i += 1;
        }
      }
      sort(tmp, (tmp + v_cnt));
      v_cnt = (unique(tmp, (tmp + v_cnt)) - tmp);
      {
        var i: dynamic = int_cpp(0);
        while ((i < int_cpp(v_cnt)))
        {
          a[i] = tmp[i].second;
          i += 1;
        }
      }
    }
  var bounds: dynamic = cpp_uninitialized();
  func flip(x: dynamic) -> dynamic
  {
      if (bounds.count(x))
      {
        bounds.erase(x);
      } else
      {
        bounds.insert(x);
      }
    }
  func solve() -> dynamic
  {
      {
        var i: dynamic = int_cpp(0);
        while ((i < int_cpp(n)))
        {
          a[cpp_update(v_cnt, "++")] = st[i];
          a[cpp_update(v_cnt, "++")] = fn[i];
          i += 1;
        }
      }
      sort_unique();
      {
        var i: dynamic = (int_cpp((v_cnt - 1)) - 1);
        while ((i >= int_cpp(0)))
        {
          a[cpp_update(v_cnt, "++")] = common(a[i], a[(i + 1)]);
          i -= 1;
        }
      }
      sort_unique();
      {
        var i: dynamic = int_cpp(0);
        while ((i < int_cpp(v_cnt)))
        {
          num[a[i]] = i;
          lvl[i] = (a[i].first + a[i].second);
          i += 1;
        }
      }
      {
        var q: dynamic = cpp_uninitialized();
        q.emplace_back(0);
        pr[0] = -1;
        {
          var i: dynamic = int_cpp(1);
          while ((i < int_cpp(v_cnt)))
          {
            while ((common(a[q.back()], a[i]) != a[q.back()]))
            {
              q.pop_back();
              assert((cpp_cast((q).size())));
            }
            pr[i] = q.back();
            q.emplace_back(i);
            i += 1;
          }
        }
      }
      memset(inc, 0, cpp_sizeof(inc));
      memset(dec, 0, cpp_sizeof(dec));
      {
        var i: dynamic = int_cpp(0);
        while ((i < int_cpp(n)))
        {
          inc[num[st[i]]] += 1;
          inc[num[fn[i]]] += 1;
          dec[num[common(st[i], fn[i])]] += 2;
          i += 1;
        }
      }
      {
        var i: dynamic = (int_cpp(v_cnt) - 1);
        while ((i >= int_cpp(0)))
        {
          if (inc[i])
          {
            flip(lvl[i]);
            flip((lvl[i] + 1));
          }
          inc[i] -= dec[i];
          if (inc[i])
          {
            assert(i);
            flip((lvl[pr[i]] + 1));
            flip(lvl[i]);
            inc[pr[i]] += inc[i];
          }
          i -= 1;
        }
      }
      bounds.erase(0);
      ans = (cpp_cast((bounds).size()));
    }
  func clear() -> dynamic
  {
      (*self) = Solution();
    }
}

var sol: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  cout.setf((ios.showpoint | ios.fixed));
  cout.precision(20);
  sol.read();
  sol.solve();
  sol.write();
  return 0;
}

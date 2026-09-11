// Translated from solution.cpp.

var pDec: dynamic = [1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000];

var fct: dynamic = [40320, 5040, 720, 120, 24, 6, 2, 1];

var dif: dynamic = [-1, 1, -3, 3];

func makeHash(num: dynamic) -> dynamic
{
  var ans: dynamic = 0;
  var f: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < 8))
    {
      var tmp: dynamic = (num % 10);
      ans += (((tmp - builtin_popcount((f & (((1 << tmp)) - 1))))) * fct[i]);
      f |= ((1 << tmp));
      num /= 10;
      i += 1;
    }
  }
  return ans;
}

func main(argument_0: dynamic) -> dynamic
{
  var vDigit: dynamic = [0, 1, 2, 3, 4, 5, 6, 7, 8];
  var h: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  while (cpp_comma(((cin >> h) >> v), (h || v)))
  {
    var hv: dynamic = [h, h, v, v];
    var st: dynamic = 0;
    var end: dynamic = 0;
    var a: dynamic = cpp_uninitialized();
    var visited: dynamic = cpp_construct(362880, false);
    read(st);
    {
      var i: dynamic = 0;
      while ((i < 8))
      {
        st *= 10;
        read(a);
        st += a;
        i += 1;
      }
    }
    read(end);
    {
      var i: dynamic = 0;
      while ((i < 8))
      {
        end *= 10;
        read(a);
        end += a;
        i += 1;
      }
    }
    var q: dynamic = cpp_uninitialized();
    q.emplace(0, st);
    while (1)
    {
      var pos: dynamic = q.top().second;
      var cost: dynamic = q.top().first;
      if ((pos == end))
      {
        write((-cost), "\n");
        break;
      }
      q.pop();
      var hash: dynamic = makeHash(pos);
      if (visited[hash])
      {
        continue;
      }
      visited[hash] = true;
      var zero: dynamic = (to_string((pos + 1000000000)).find(cpp_char("0")) - 1);
      {
        var i: dynamic = 0;
        while ((i < 4))
        {
          var s: dynamic = pos;
          var t: dynamic = (8 - (((((zero + dif[i]) + 9)) % 9)));
          var tmp: dynamic = (((pos / pDec[t])) % 10);
          s += (((pDec[(8 - zero)] * tmp) - (pDec[t] * tmp)));
          q.emplace((cost - hv[i]), s);
          i += 1;
        }
      }
    }
  }
  return 0;
}

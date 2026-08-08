// Translated from solution.cpp.

var pDec = [1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000];

var fct = [40320, 5040, 720, 120, 24, 6, 2, 1];

var dif = [-1, 1, -3, 3];

func makeHash(num: dynamic)
{
  var ans = 0;
  var f = 0;
  {
    var i = 0;
    while ((i < 8))
    {
      var tmp = (num % 10);
      ans += (((tmp - builtin_popcount((f & (((1 << tmp)) - 1))))) * fct[i]);
      f |= ((1 << tmp));
      num /= 10;
      i += 1;
    }
  }
  return ans;
}

func main(argument_0: dynamic)
{
  var vDigit = [0, 1, 2, 3, 4, 5, 6, 7, 8];
  var h: dynamic;
  var v: dynamic;
  while (cpp_comma(((cin >> h) >> v), (h || v)))
  {
    var hv = [h, h, v, v];
    var st = 0;
    var end = 0;
    var a: dynamic;
    var visited = cpp_construct(362880, false);
    read(st);
    {
      var i = 0;
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
      var i = 0;
      while ((i < 8))
      {
        end *= 10;
        read(a);
        end += a;
        i += 1;
      }
    }
    var q: dynamic;
    q.emplace(0, st);
    while (1)
    {
      var pos = q.top().second;
      var cost = q.top().first;
      if ((pos == end))
      {
        write((-cost), "\n");
        break;
      }
      q.pop();
      var hash = makeHash(pos);
      if (visited[hash])
      {
        continue;
      }
      visited[hash] = true;
      var zero = (to_string((pos + 1000000000)).find(cpp_char("0")) - 1);
      {
        var i = 0;
        while ((i < 4))
        {
          var s = pos;
          var t = (8 - (((((zero + dif[i]) + 9)) % 9)));
          var tmp = (((pos / pDec[t])) % 10);
          s += (((pDec[(8 - zero)] * tmp) - (pDec[t] * tmp)));
          q.emplace((cost - hv[i]), s);
          i += 1;
        }
      }
    }
  }
  return 0;
}

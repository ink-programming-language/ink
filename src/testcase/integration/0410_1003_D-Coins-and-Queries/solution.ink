// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  var coins: dynamic = cpp_uninitialized();
  read(n, q);
  var queries: dynamic = cpp_array(q);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var val: dynamic = cpp_uninitialized();
      read(val);
      coins[val] += 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      read(queries[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      var counter: dynamic = 0;
      var curr: dynamic = 0;
      {
        var it: dynamic = coins.rbegin();
        while ((it != coins.rend()))
        {
          var num: dynamic = min(it->second, (((queries[i] - curr)) / (it->first)));
          counter += num;
          curr += (num * (it->first));
          it += 1;
        }
      }
      if ((curr == queries[i]))
      {
        write(counter, "\n");
      } else
      {
        write(-1, "\n");
      }
      i += 1;
    }
  }
}

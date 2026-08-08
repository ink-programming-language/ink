// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var q: dynamic;
  var coins: dynamic;
  read(n, q);
  var queries = cpp_array(q);
  {
    var i = 0;
    while ((i < n))
    {
      var val: dynamic;
      read(val);
      coins[val] += 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < q))
    {
      read(queries[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < q))
    {
      var counter = 0;
      var curr = 0;
      {
        var it = coins.rbegin();
        while ((it != coins.rend()))
        {
          var num = min(it->second, (((queries[i] - curr)) / (it->first)));
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

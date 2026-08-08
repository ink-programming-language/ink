// Translated from solution.cpp.

var b: dynamic;

var ans: dynamic;

var cnt: dynamic;

var sum2: dynamic;

func num_to_string(num: dynamic)
{
  var ss: dynamic;
  (ss << num);
  return ss.str();
}

func O_o()
{
  ios.sync_with_stdio(0);
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
}

func main()
{
  O_o();
  var n: dynamic;
  var k: dynamic;
  var s: dynamic;
  read(n, k);
  {
    var i = 0;
    while ((i < n))
    {
      read(s);
      var ss: dynamic;
      b = 0;
      {
        var j = 0;
        while ((j < (cpp_cast((s.size())))))
        {
          ss.insert((s[j] - cpp_char("0")));
          j += 1;
        }
      }
      var x = 0;
      for (var j in ss)
      {
        if ((j != x))
        {
          b = 1;
        }
        if ((x == k))
        {
          break;
        }
        x += 1;
      }
      if ((((cpp_cast((ss.size()))) < (k + 1)) || (x != k)))
      {
        b = 1;
      }
      if ((!b))
      {
        cnt += 1;
      }
      i += 1;
    }
  }
  write(cnt, "\n");
  return 0;
}

// Translated from solution.cpp.

func ITS(x: dynamic)
{
  var s = "";
  while ((x > 0))
  {
    s += cpp_cast((((x % 10) + cpp_char("0"))));
    x /= 10;
  }
  var t = "";
  {
    var i = (s.size() - 1);
    while ((i > -1))
    {
      t += s[i];
      i -= 1;
    }
  }
  return t;
}

var inf = 1e9;

var mod = (1e9 + 7);

var M = 2e5;

var online = cpp_array(M);

var t = cpp_array(M);

func main()
{
  cin.sync_with_stdio(false);
  var n: dynamic;
  var k: dynamic;
  var q: dynamic;
  read(n, k, q);
  {
    var i = 1;
    while ((i <= n))
    {
      read(t[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < q))
    {
      var qt: dynamic;
      var qid: dynamic;
      read(qt, qid);
      if ((qt == 1))
      {
        sort(online, (online + k));
        if ((t[qid] > online[0].first))
        {
          online[0].first = t[qid];
          online[0].second = qid;
        }
      } else
      {
        var i: dynamic;
        {
          i = 0;
          while ((i < k))
          {
            if ((online[i].second == qid))
            {
              write("YES\n");
              break;
            }
            i += 1;
          }
        }
        if ((i == k))
        {
          write("NO\n");
        }
      }
      i += 1;
    }
  }
}

// Translated from solution.cpp.

func ITS(x: dynamic) -> dynamic
{
  var s: dynamic = "";
  while ((x > 0))
  {
    s += cpp_cast((((x % 10) + cpp_char("0"))));
    x /= 10;
  }
  var t: dynamic = "";
  {
    var i: dynamic = (s.size() - 1);
    while ((i > -1))
    {
      t += s[i];
      i -= 1;
    }
  }
  return t;
}

var inf: dynamic = 1e9;

var mod: dynamic = (1e9 + 7);

var M: dynamic = 2e5;

var online: dynamic = cpp_array(M);

var t: dynamic = cpp_array(M);

func main() -> dynamic
{
  cin.sync_with_stdio(false);
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  read(n, k, q);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(t[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      var qt: dynamic = cpp_uninitialized();
      var qid: dynamic = cpp_uninitialized();
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
        var i: dynamic = cpp_uninitialized();
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

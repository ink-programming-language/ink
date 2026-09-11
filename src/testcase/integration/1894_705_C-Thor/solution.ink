// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  var unread: dynamic = 0;
  var clearupto: dynamic = 0;
  var t: dynamic = 1;
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, q);
  var app: dynamic = cpp_construct((n + 1));
  var time: dynamic = cpp_construct((q + 1));
  var it: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  while (cpp_update(q, "--"))
  {
    read(x, y);
    if ((x == 1))
    {
      app[y].push_back(t);
      time[cpp_update(t, "++")] = y;
      unread += 1;
    } else if ((x == 2))
    {
      it = app[y].begin();
      while ((it != app[y].end()))
      {
        if (((*it) <= clearupto))
        {
        } else
        {
          time[(*it)] = 0;
          unread -= 1;
        }
        it += 1;
      }
      app[y].clear();
    } else
    {
      {
        j = (clearupto + 1);
        while ((j <= y))
        {
          if ((time[j] != 0))
          {
            unread -= 1;
          }
          j += 1;
        }
      }
      if ((y > clearupto))
      {
        clearupto = y;
      }
    }
    write(unread, "\n");
  }
  return 0;
}

// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic;
  var q: dynamic;
  var unread = 0;
  var clearupto = 0;
  var t = 1;
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  read(n, q);
  var app = cpp_construct((n + 1));
  var time = cpp_construct((q + 1));
  var it: dynamic;
  var x: dynamic;
  var y: dynamic;
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

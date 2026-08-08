// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  var m: dynamic;
  var num = 1;
  var com: dynamic;
  var arg: dynamic;
  var d: dynamic;
  read(t, m);
  {
    var n = cpp_construct(((t) - 1));
    var q = cpp_construct(0);
    while ((q <= n))
    {
      sort((d).begin(), (d).end());
      read(com);
      if ((com == "defragment"))
      {
        if ((d.size() > 0))
        {
          d[0].first.second -= d[0].first.first;
          d[0].first.first = 0;
          {
            var n = cpp_construct(((d.size() - 1)));
            var i = cpp_construct(1);
            while ((i <= n))
            {
              var delta = (d[i].first.first - d[(i - 1)].first.second);
              d[i].first.first -= delta;
              d[i].first.second -= delta;
              i += 1;
            }
          }
        }
        q += 1;
        continue;
      }
      read(arg);
      if ((com == "alloc"))
      {
        var res = -1;
        if ((d.size() > 0))
        {
          if ((d[0].first.first >= arg))
          {
            d.push_back(make_pair(make_pair(0, arg), cpp_update(num, "++")));
            res = 1;
          }
        }
        if ((res == -1))
        {
          {
            var n = cpp_construct(((d.size() - 1)));
            var i = cpp_construct(1);
            while ((i <= n))
            {
              if (((d[i].first.first - d[(i - 1)].first.second) >= arg))
              {
                d.push_back(make_pair(make_pair(d[(i - 1)].first.second, (d[(i - 1)].first.second + arg)), cpp_update(num, "++")));
                res = 1;
                break;
              }
              i += 1;
            }
          }
        }
        if ((res == -1))
        {
          if ((d.size() > 0))
          {
            if (((m - d[(d.size() - 1)].first.second) >= arg))
            {
              d.push_back(make_pair(make_pair(d[(d.size() - 1)].first.second, (d[(d.size() - 1)].first.second + arg)), cpp_update(num, "++")));
              res = 1;
            }
          }
        }
        if ((res == -1))
        {
          if ((d.size() == 0))
          {
            if ((m >= arg))
            {
              d.push_back(make_pair(make_pair(0, arg), cpp_update(num, "++")));
              res = 1;
            }
          }
        }
        if ((res == -1))
        {
          write("NULL", "\n");
        } else
        {
          write((num - 1), "\n");
        }
        q += 1;
        continue;
      }
      if ((com == "erase"))
      {
        var res = false;
        {
          var i = 0;
          while ((i < d.size()))
          {
            if ((d[i].second == arg))
            {
              swap(d[i], d[(d.size() - 1)]);
              d.pop_back();
              res = true;
              break;
            }
            i += 1;
          }
        }
        if ((!res))
        {
          write("ILLEGAL_ERASE_ARGUMENT", "\n");
        }
        q += 1;
        continue;
      }
      q += 1;
    }
  }
  return 0;
}

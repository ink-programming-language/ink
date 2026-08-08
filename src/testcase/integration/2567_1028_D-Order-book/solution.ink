// Translated from solution.cpp.

var mod = (1e9 + 7);

func main()
{
  srand(time(null));
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic;
  read(n);
  var mis1: dynamic;
  var mis2: dynamic;
  var we: dynamic;
  var ans = 1;
  {
    var i = 0;
    while ((i < n))
    {
      var s: dynamic;
      read(s);
      if ((s == "ADD"))
      {
        var a: dynamic;
        read(a);
        we.push_back(a);
      } else
      {
        var a: dynamic;
        read(a);
        sort(we.begin(), we.end());
        if ((mis2.size() != 0))
        {
          while ((we.size() && (we.back() > ((*mis2.begin())))))
          {
            mis2.insert(we.back());
            we.pop_back();
          }
        }
        if ((mis1.size() != 0))
        {
          reverse(we.begin(), we.end());
          while ((we.size() && (we.back() < (*(cpp_update(mis1.end(), "--"))))))
          {
            mis1.insert(we.back());
            we.pop_back();
          }
          reverse(we.begin(), we.end());
        }
        if ((((mis1.size() && (a < (*(cpp_update(mis1.end(), "--")))))) || ((mis2.size() && (a > ((*mis2.begin())))))))
        {
          write("0");
          return 0;
        }
        if ((mis1.size() && (a == (*(cpp_update(mis1.end(), "--"))))))
        {
          {
            var i = 0;
            while ((i < we.size()))
            {
              mis2.insert(we[i]);
              i += 1;
            }
          }
          we.clear();
          mis1.erase(cpp_update(mis1.end(), "--"));
        } else if ((mis2.size() && (a == ((*mis2.begin())))))
        {
          {
            var i = 0;
            while ((i < we.size()))
            {
              mis1.insert(we[i]);
              i += 1;
            }
          }
          we.clear();
          mis2.erase(mis2.begin());
        } else
        {
          {
            var i = 0;
            while ((i < we.size()))
            {
              if ((we[i] < a))
              {
                mis1.insert(we[i]);
              } else if ((we[i] > a))
              {
                mis2.insert(we[i]);
              }
              i += 1;
            }
          }
          we.clear();
          ans = (((ans * 2)) % mod);
        }
      }
      i += 1;
    }
  }
  sort(we.begin(), we.end());
  if ((mis2.size() != 0))
  {
    while ((we.size() && (we.back() > ((*mis2.begin())))))
    {
      mis2.insert(we.back());
      we.pop_back();
    }
  }
  if ((mis1.size() != 0))
  {
    reverse(we.begin(), we.end());
    while ((we.size() && (we.back() < (*(cpp_update(mis1.end(), "--"))))))
    {
      mis1.insert(we.back());
      we.pop_back();
    }
    reverse(we.begin(), we.end());
  }
  if (we.size())
  {
    ans = (((ans * ((cpp_cast(we.size()) + 1)))) % mod);
  }
  write(ans);
}

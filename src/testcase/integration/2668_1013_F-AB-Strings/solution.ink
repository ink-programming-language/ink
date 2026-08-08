// Translated from solution.cpp.

class Solution
{
  func construct(s: dynamic, t: dynamic)
  {
      var qs: dynamic;
      var qt: dynamic;
      var n = s.length();
      var m = t.length();
      {
        var i = 0;
        while ((i < n))
        {
          var c = s[i];
          var cnt = 1;
          while ((((i + 1) < n) && (s[(i + 1)] == c)))
          {
            i += 1;
            cnt += 1;
          }
          qs.emplace_back(c, cnt);
          i += 1;
        }
      }
      {
        var i = 0;
        while ((i < m))
        {
          var c = t[i];
          var cnt = 1;
          while ((((i + 1) < m) && (t[(i + 1)] == c)))
          {
            i += 1;
            cnt += 1;
          }
          qt.emplace_back(c, cnt);
          i += 1;
        }
      }
      var res: dynamic;
      res.reserve(((n + m) + 1000));
      while (((qs.size() != 1) || (qt.size() != 1)))
      {
        assert(((qs.size() > 0) && (qt.size() > 0)));
        if (qs.empty())
        {
          var n = (qt.size() / 2);
          if ((((n % 2) == 0) && ((n + 1) < qt.size())))
          {
            n += 1;
          }
          var cnt = 0;
          while ((cpp_update(n, "--") > 0))
          {
            cnt += qt.front().second;
            qs.push_back(qt.front());
            qt.pop_front();
          }
          res.emplace_back(0, cnt);
        } else if (qt.empty())
        {
          var n = (qs.size() / 2);
          if ((((n % 2) == 0) && ((n + 1) < qs.size())))
          {
            n += 1;
          }
          var cnt = 0;
          while ((cpp_update(n, "--") > 0))
          {
            cnt += qs.front().second;
            qt.push_back(qs.front());
            qs.pop_front();
          }
          res.emplace_back(cnt, 0);
        } else if ((qs.size() == 1))
        {
          var n = (qt.size() / 2);
          if ((((n % 2) == 0) && ((n + 1) < qt.size())))
          {
            n += 1;
          }
          var cnt = 0;
          var c = qs.front().first;
          var cnt1 = qs.front().second;
          qs.pop_front();
          while ((cpp_update(n, "--") > 0))
          {
            cnt += qt.front().second;
            if ((n == 0))
            {
              if ((c == qt.front().first))
              {
                qs.emplace_back(c, (qt.front().second + cnt1));
                qt.pop_front();
                cnt1 = 0;
              } else
              {
                qs.push_back(qt.front());
                qt.pop_front();
                qt.front().second += cnt1;
              }
              break;
            } else
            {
              qs.push_back(qt.front());
            }
            qt.pop_front();
          }
          res.emplace_back(cnt1, cnt);
        } else if ((qt.size() == 1))
        {
          var n = (qs.size() / 2);
          if ((((n % 2) == 0) && ((n + 1) < qs.size())))
          {
            n += 1;
          }
          var cnt = 0;
          var c = qt.front().first;
          var cnt1 = qt.front().second;
          qt.pop_front();
          while ((cpp_update(n, "--") > 0))
          {
            cnt += qs.front().second;
            if ((n == 0))
            {
              if ((qs.front().first == c))
              {
                qt.emplace_back(c, (qs.front().second + cnt1));
                qs.pop_front();
                cnt1 = 0;
              } else
              {
                qt.push_back(qs.front());
                qs.pop_front();
                qs.front().second += cnt1;
              }
              break;
            } else
            {
              qt.push_back(qs.front());
            }
            qs.pop_front();
          }
          res.emplace_back(cnt, cnt1);
        } else if ((qs.front().first == qt.front().first))
        {
          if (((qs.size() >= 2) && (qt.size() >= 2)))
          {
            if ((qs.size() >= qt.size()))
            {
              var p1 = qs.front();
              qs.pop_front();
              var p2 = qs.front();
              qs.pop_front();
              var p3 = qt.front();
              qt.pop_front();
              if (qs.empty())
              {
                qs.push_front(p3);
              } else
              {
                qs.front().second += p3.second;
              }
              qt.front().second += p2.second;
              qt.push_front(p1);
              res.emplace_back((p1.second + p2.second), p3.second);
            } else
            {
              var p1 = qs.front();
              qs.pop_front();
              var p2 = qt.front();
              qt.pop_front();
              var p3 = qt.front();
              qt.pop_front();
              qs.front().second += p3.second;
              qs.push_front(p2);
              qt.front().second += p1.second;
              res.emplace_back(p1.second, (p2.second + p3.second));
            }
          } else
          {
            if ((qs.size() >= qt.size()))
            {
              res.emplace_back(qs.front().second, 0);
              qt.front().second += qs.front().second;
              qs.pop_front();
            } else
            {
              res.emplace_back(0, qt.front().second);
              qs.front().second += qt.front().second;
              qt.pop_front();
            }
          }
        } else
        {
          if (((qs.size() > 3) && (qt.size() == 2)))
          {
            var p1 = qs.front();
            qs.pop_front();
            var p2 = qs.front();
            qs.pop_front();
            var p3 = qs.front();
            qs.pop_front();
            var p4 = qt.front();
            qt.pop_front();
            res.emplace_back(((p1.second + p2.second) + p3.second), p4.second);
            qs.front().second += p4.second;
            qt.front().second += p3.second;
            qt.push_front(p2);
            qt.push_front(p1);
            continue;
          } else if (((qs.size() == 2) && (qt.size() > 3)))
          {
            var p1 = qs.front();
            qs.pop_front();
            var p2 = qt.front();
            qt.pop_front();
            var p3 = qt.front();
            qt.pop_front();
            var p4 = qt.front();
            qt.pop_front();
            res.emplace_back(p1.second, ((p2.second + p3.second) + p4.second));
            qs.front().second += p4.second;
            qs.push_front(p3);
            qs.push_front(p2);
            qt.front().second += p1.second;
            continue;
          }
          var cnt = qs.front().second;
          var cnt1 = qt.front().second;
          res.emplace_back(cnt, cnt1);
          qs.pop_front();
          qt.pop_front();
          qs.front().second += cnt1;
          qt.front().second += cnt;
        }
      }
      return res;
    }
}

func main(argc: dynamic, argv: dynamic)
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var s: dynamic;
  var t: dynamic;
  read(s);
  read(t);
  var sol: dynamic;
  var res = sol.construct(s, t);
  write(res.size(), "\n");
  for (var p in res)
  {
    write(p.first, " ", p.second, "\n");
  }
  return 0;
}

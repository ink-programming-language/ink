// Translated from solution.cpp.

var N = 400000;

var a = cpp_array(N);

var t = cpp_array(N);

var type_cpp = cpp_array(N);

var us = cpp_array(N);

func main()
{
  ios.sync_with_stdio(0);
  var n: dynamic;
  var w: dynamic;
  var k: dynamic;
  read(n, w, k);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      read(t[i]);
      i += 1;
    }
  }
  var r = -1;
  var curr_time = 0;
  var earn = 0;
  var half: dynamic;
  var full: dynamic;
  var ans = 0;
  {
    var i = 0;
    while ((i < n))
    {
      r = max(r, (i - 1));
      while ((r < (n - 1)))
      {
        r += 1;
        var temp = curr_time;
        var pers_full: dynamic;
        var pers_half: dynamic;
        var pers_type: dynamic;
        half.insert(make_pair(t[r], r));
        pers_half.push_back(make_pair(+1, r));
        pers_type.push_back(make_pair(r, type_cpp[r]));
        type_cpp[r] = 0;
        curr_time += (((t[r] + 1)) / 2);
        if ((half.size() > w))
        {
          var p = (*half.begin());
          curr_time -= (((p.first + 1)) / 2);
          half.erase(half.find(p));
          pers_half.push_back(make_pair(-1, p.second));
          curr_time += p.first;
          full.insert(p);
          pers_full.push_back(make_pair(+1, p.second));
          pers_type.push_back(make_pair(p.second, type_cpp[p.second]));
          type_cpp[p.second] = 1;
        }
        if ((curr_time > k))
        {
          while (pers_full.size())
          {
            var p = pers_full.back();
            pers_full.pop_back();
            if ((p.first == -1))
            {
              full.insert(make_pair(t[p.second], p.second));
            } else
            {
              full.erase(make_pair(t[p.second], p.second));
            }
          }
          while (pers_half.size())
          {
            var p = pers_half.back();
            pers_half.pop_back();
            if ((p.first == -1))
            {
              half.insert(make_pair(t[p.second], p.second));
            } else
            {
              half.erase(make_pair(t[p.second], p.second));
            }
          }
          while (pers_type.size())
          {
            var p = pers_type.back();
            pers_type.pop_back();
            type_cpp[p.first] = p.second;
          }
          curr_time = temp;
          r -= 1;
          break;
        } else
        {
          earn += a[r];
          us[r] = 1;
        }
      }
      ans = max(earn, ans);
      if ((us[i] == 1))
      {
        if ((type_cpp[i] == 0))
        {
          half.erase(make_pair(t[i], i));
          curr_time -= (((t[i] + 1)) / 2);
        } else
        {
          full.erase(make_pair(t[i], i));
          curr_time -= t[i];
        }
        earn -= a[i];
        us[i] = 0;
        while ((full.size() && (half.size() < w)))
        {
          var p = (*full.rbegin());
          full.erase(full.find(p));
          half.insert(p);
          curr_time -= p.first;
          curr_time += (((p.first + 1)) / 2);
          type_cpp[p.second] = 0;
        }
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}

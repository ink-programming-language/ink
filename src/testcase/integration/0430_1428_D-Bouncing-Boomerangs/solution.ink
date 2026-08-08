// Translated from solution.cpp.

func print_iterable(begin_iter: dynamic, end_iter: dynamic, counter: dynamic)
{
  var done_something = false;
  var res: dynamic;
  (res << "[");
  {
    while (cpp_binary((begin_iter != end_iter), "and", counter))
    {
      done_something = true;
      counter -= 1;
      ((res << (*begin_iter)) << ", ");
      begin_iter += 1;
    }
  }
  var str = res.str();
  if (done_something)
  {
    str.pop_back();
    str.pop_back();
  }
  str += "]";
  return str;
}

func SortIndex(size: dynamic, compare: dynamic)
{
  {
    var i = 0;
    while ((i < size))
    {
      ord[i] = i;
      i += 1;
    }
  }
  sort(ord.begin(), ord.end(), compare);
  return ord;
}

func MinPlace(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
    return true;
  }
  return false;
}

func MaxPlace(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
    return true;
  }
  return false;
}

func operator_shift_left(out: dynamic, p: dynamic)
{
  (((((out << "{") << p.first) << ", ") << p.second) << "}");
  return out;
}

func operator_shift_left(out: dynamic, v: dynamic)
{
  (out << "[");
  {
    var i = 0;
    while ((i < cpp_cast(v.size())))
    {
      (out << v[i]);
      if ((i != (cpp_cast(v.size()) - 1)))
      {
        (out << ", ");
      }
      i += 1;
    }
  }
  (out << "]");
  return out;
}

func dbg(name: dynamic, val: dynamic)
{
  write(name, ": ", val, "\n");
}

func dbg(names: dynamic, curr_val: dynamic, vals: dynamic...)
{
  while (((*names) != cpp_char(",")))
  {
    write((*cpp_update(names, "++")));
  }
  write(": ", curr_val, ", ");
  dbg((names + 1), cpp_expand(vals));
}

var MAXN = (1e5 + 100);

var a = cpp_array(MAXN);

var ans: dynamic;

var n: dynamic;

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var h_free = 1;
  var req = cpp_array(4);
  {
    var i = 1;
    while ((i <= n))
    {
      if ((a[i] == 0))
      {
        i += 1;
        continue;
      }
      if ((a[i] == 1))
      {
        if ((req[2].empty() && req[3].empty()))
        {
          ans.emplace_back(i, h_free);
          h_free += 1;
        } else
        {
          var cima: dynamic;
          if (req[2].empty())
          {
            cima = req[3].back();
            req[3].pop_back();
          } else
          {
            cima = req[2].back();
            req[2].pop_back();
          }
          ans.emplace_back(i, cima.second);
          if ((cima.first == 3))
          {
            ans.emplace_back(i, h_free);
            h_free += 1;
          }
        }
      } else
      {
        if ((!req[3].empty()))
        {
          var cima = req[3].back();
          req[3].pop_back();
          ans.emplace_back(i, cima.second);
        }
        ans.emplace_back(i, h_free);
        req[a[i]].emplace_back(a[i], h_free);
        h_free += 1;
      }
      i += 1;
    }
  }
  if (((!req[2].empty()) || (!req[3].empty())))
  {
    write(-1, cpp_char("\n"));
  } else
  {
    write(ans.size(), cpp_char("\n"));
    for (var el in ans)
    {
      write(el.second, " ", el.first, cpp_char("\n"));
    }
  }
  return 0;
}

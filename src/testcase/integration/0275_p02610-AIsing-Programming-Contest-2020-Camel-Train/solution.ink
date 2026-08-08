// Translated from solution.cpp.

func solve()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(k.at(i), l.at(i), r.at(i));
      i += 1;
    }
  }
  var s = 0;
  var al: dynamic;
  var ar: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      s += min(l.at(i), r.at(i));
      if ((l.at(i) > r.at(i)))
      {
        al.emplace_back(k.at(i), (max(l.at(i), r.at(i)) - min(l.at(i), r.at(i))));
      } else
      {
        ar.emplace_back((n - k.at(i)), (max(l.at(i), r.at(i)) - min(l.at(i), r.at(i))));
      }
      i += 1;
    }
  }
  sort(al.rbegin(), al.rend());
  sort(ar.rbegin(), ar.rend());
  var q: dynamic;
  var ans = s;
  var j = 0;
  {
    var i = 0;
    while ((i < n))
    {
      while (cpp_binary((j < al.size()), "and", (al.at(j).first == (n - i))))
      {
        q.push(al.at(j).second);
        j += 1;
      }
      if (q.empty())
      {
        i += 1;
        continue;
      }
      ans += q.top();
      q.pop();
      i += 1;
    }
  }
  while ((!q.empty()))
  {
    q.pop();
  }
  j = 0;
  {
    var i = 0;
    while ((i < n))
    {
      while (cpp_binary((j < ar.size()), "and", (ar.at(j).first == (n - i))))
      {
        q.push(ar.at(j).second);
        j += 1;
      }
      if (q.empty())
      {
        i += 1;
        continue;
      }
      ans += q.top();
      q.pop();
      i += 1;
    }
  }
  write(ans, "\n");
}

func main()
{
  var t: dynamic;
  read(t);
  {
    var i = 0;
    while ((i < t))
    {
      solve();
      i += 1;
    }
  }
}

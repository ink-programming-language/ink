// Translated from solution.cpp.

func main()
{
  var arr = [4, 8, 15, 16, 23, 42];
  var v: dynamic;
  var ans = cpp_array(7);
  {
    var i = 0;
    while ((i < 5))
    {
      {
        var j = (i + 1);
        while ((j < 6))
        {
          v.push_back(make_pair((arr[i] * arr[j]), make_pair(arr[i], arr[j])));
          j += 1;
        }
      }
      i += 1;
    }
  }
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
  write("? ", 1, " ", 2, "\n");
  fflush(stdout);
  read(a);
  fflush(stdout);
  var p1: dynamic;
  var p2: dynamic;
  {
    var i = 0;
    while ((i < v.size()))
    {
      if ((v[i].first == a))
      {
        p1 = v[i];
        break;
      }
      i += 1;
    }
  }
  write("? ", 2, " ", 3, "\n");
  fflush(stdout);
  read(b);
  fflush(stdout);
  {
    var i = 0;
    while ((i < v.size()))
    {
      if ((v[i].first == b))
      {
        p2 = v[i];
        break;
      }
      i += 1;
    }
  }
  if ((p1.second.first == p2.second.first))
  {
    ans[2] = p1.second.first;
    ans[1] = p1.second.second;
    ans[3] = p2.second.second;
  } else if ((p1.second.first == p2.second.second))
  {
    ans[2] = p1.second.first;
    ans[1] = p1.second.second;
    ans[3] = p2.second.first;
  } else if ((p1.second.second == p2.second.first))
  {
    ans[2] = p1.second.second;
    ans[1] = p1.second.first;
    ans[3] = p2.second.second;
  } else if ((p1.second.second == p2.second.second))
  {
    ans[2] = p1.second.second;
    ans[1] = p1.second.first;
    ans[3] = p2.second.first;
  }
  write("? ", 4, " ", 5, "\n");
  fflush(stdout);
  read(c);
  fflush(stdout);
  var p3: dynamic;
  var p4: dynamic;
  {
    var i = 0;
    while ((i < v.size()))
    {
      if ((v[i].first == c))
      {
        p3 = v[i];
        break;
      }
      i += 1;
    }
  }
  write("? ", 5, " ", 6, "\n");
  fflush(stdout);
  read(d);
  fflush(stdout);
  {
    var i = 0;
    while ((i < v.size()))
    {
      if ((v[i].first == d))
      {
        p4 = v[i];
        break;
      }
      i += 1;
    }
  }
  if ((p3.second.first == p4.second.first))
  {
    ans[5] = p3.second.first;
    ans[4] = p3.second.second;
    ans[6] = p4.second.second;
  } else if ((p3.second.first == p4.second.second))
  {
    ans[5] = p3.second.first;
    ans[4] = p3.second.second;
    ans[6] = p4.second.first;
  } else if ((p3.second.second == p4.second.first))
  {
    ans[5] = p3.second.second;
    ans[4] = p3.second.first;
    ans[6] = p4.second.second;
  } else if ((p3.second.second == p4.second.second))
  {
    ans[5] = p3.second.second;
    ans[4] = p3.second.first;
    ans[6] = p4.second.first;
  }
  write("! ");
  {
    var i = 1;
    while ((i <= 6))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
  return 0;
}

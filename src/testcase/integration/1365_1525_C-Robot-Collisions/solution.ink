// Translated from solution.cpp.

var mod: dynamic = (1e9 + 7);

var MAXN: dynamic = cpp_expression("#include");

var spf: dynamic = cpp_array(MAXN);

var ans: dynamic = cpp_construct(3e5, 0);

var m: dynamic = cpp_uninitialized();

func func_cpp(v: dynamic) -> dynamic
{
  var st: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < v.size()))
    {
      if ((st.size() == 0))
      {
        st.push_back(v[i]);
      } else if (((get(st.back()) == cpp_char("R")) && (get(v[i]) == cpp_char("L"))))
      {
        ans[get(st.back())] = (abs((get(st.back()) - get(v[i]))) / 2);
        ans[get(v[i])] = (abs((get(st.back()) - get(v[i]))) / 2);
        st.pop_back();
      } else
      {
        st.push_back(v[i]);
      }
      i += 1;
    }
  }
  var vl: dynamic = cpp_uninitialized();
  var vr: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < st.size()))
    {
      if ((get(st[i]) == cpp_char("L")))
      {
        vl.push_back(st[i]);
      } else
      {
        vr.push_back(st[i]);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < vl.size()))
    {
      ans[get(vl[i])] = (abs((get(vl[i]) + get(vl[(i - 1)]))) / 2);
      ans[get(vl[(i - 1)])] = (abs((get(vl[i]) + get(vl[(i - 1)]))) / 2);
      i = (i + 2);
    }
  }
  {
    var i: dynamic = (vr.size() - 1);
    while ((i > 0))
    {
      ans[get(vr[i])] = (((abs((get(vr[i]) - get(vr[(i - 1)]))) / 2) + m) - get(vr[i]));
      ans[get(vr[(i - 1)])] = (((abs((get(vr[i]) - get(vr[(i - 1)]))) / 2) + m) - get(vr[i]));
      i = (i - 2);
    }
  }
  if ((((vl.size() % 2) == 1) && ((vr.size() % 2) == 1)))
  {
    ans[get(vr[0])] = ((((get(vl[(vl.size() - 1)]) - get(vr[0]))) / 2) + m);
    ans[get(vl[(vl.size() - 1)])] = ((((get(vl[(vl.size() - 1)]) - get(vr[0]))) / 2) + m);
  } else if ((vl.size() % 2))
  {
    ans[get(vl[(vl.size() - 1)])] = -1;
  } else if ((vr.size() % 2))
  {
    ans[get(vr[0])] = -1;
  }
}

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    read(n, m);
    var odd: dynamic = cpp_uninitialized();
    var even: dynamic = cpp_uninitialized();
    var a: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        var x: dynamic = cpp_uninitialized();
        read(x);
        a.push_back(x);
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        var c: dynamic = cpp_uninitialized();
        read(c);
        var p: dynamic = cpp_uninitialized();
        get(p) = a[i];
        get(p) = c;
        get(p) = i;
        if ((a[i] % 2))
        {
          odd.push_back(p);
        } else
        {
          even.push_back(p);
        }
        i += 1;
      }
    }
    sort(odd.begin(), odd.end());
    sort(even.begin(), even.end());
    func_cpp(odd);
    func_cpp(even);
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        write(ans[i], " ");
        i += 1;
      }
    }
    write("\n");
  }
}

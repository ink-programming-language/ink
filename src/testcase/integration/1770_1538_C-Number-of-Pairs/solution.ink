// Translated from solution.cpp.

var ll: dynamic = dynamic;

var pb: dynamic = cpp_expression("#include<");

var vi: dynamic = cpp_expression("#include<bi");

var vii: dynamic = cpp_expression("#include<bits/stdc++.h");

var vll: dynamic = cpp_expression("#include<b");

var F: dynamic = cpp_expression("#incl");

var S: dynamic = cpp_expression("#inclu");

var fast: dynamic = cpp_expression("#include<bits/stdc++.h> #define l");

var N: dynamic = cpp_uninitialized();

var T: dynamic = cpp_uninitialized();

var ind: dynamic = cpp_uninitialized();

func query(pos: dynamic) -> dynamic
{
  var res: dynamic = 0;
  while ((pos > 0))
  {
    res += T[pos];
    pos -= ((pos & (-pos)));
  }
  return res;
}

func update(pos: dynamic) -> dynamic
{
  while ((pos <= N))
  {
    T[pos] += 1;
    pos += ((pos & (-pos)));
  }
}

func main() -> dynamic
{
  var int_cpp: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  read(t);
  var st: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  while (cpp_update(t, "--"))
  {
    v.clear();
    ind.clear();
    st.clear();
    read(n, l, r);
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(x);
        v.pb(x);
        st.insert(x);
        if (((l - x) > 0))
        {
          st.insert((l - x));
        }
        if (((r - x) > 0))
        {
          st.insert((r - x));
        }
        i += 1;
      }
    }
    var num: dynamic = 1;
    for (var it: dynamic in st)
    {
      ind[it] = num;
      num += 1;
    }
    N = num;
    T.assign((N + 2), 0);
    var res: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        x = v[i];
        var a: dynamic = max((l - x), 1);
        var b: dynamic = (r - x);
        if ((b > 0))
        {
          res += (query(ind[b]) - query((ind[a] - 1)));
        }
        update(ind[x]);
        i += 1;
      }
    }
    write(res, "\n");
  }
  return 0;
}

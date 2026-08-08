// Translated from solution.cpp.

var ll = dynamic;

var pb = cpp_expression("#include<");

var vi = cpp_expression("#include<bi");

var vii = cpp_expression("#include<bits/stdc++.h");

var vll = cpp_expression("#include<b");

var F = cpp_expression("#incl");

var S = cpp_expression("#inclu");

var fast = cpp_expression("#include<bits/stdc++.h> #define l");

var N: dynamic;

var T: dynamic;

var ind: dynamic;

func query(pos: dynamic)
{
  var res = 0;
  while ((pos > 0))
  {
    res += T[pos];
    pos -= ((pos & (-pos)));
  }
  return res;
}

func update(pos: dynamic)
{
  while ((pos <= N))
  {
    T[pos] += 1;
    pos += ((pos & (-pos)));
  }
}

func main()
{
  var int_cpp: dynamic;
  var n: dynamic;
  var l: dynamic;
  var r: dynamic;
  var x: dynamic;
  read(t);
  var st: dynamic;
  var v: dynamic;
  while (cpp_update(t, "--"))
  {
    v.clear();
    ind.clear();
    st.clear();
    read(n, l, r);
    {
      var i = 0;
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
    var num = 1;
    for (var it in st)
    {
      ind[it] = num;
      num += 1;
    }
    N = num;
    T.assign((N + 2), 0);
    var res = 0;
    {
      var i = 0;
      while ((i < n))
      {
        x = v[i];
        var a = max((l - x), 1);
        var b = (r - x);
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

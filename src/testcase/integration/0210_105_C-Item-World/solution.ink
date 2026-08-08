// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

class lut
{
  var name: dynamic;
  var cl: dynamic;
  var val: dynamic;
  var size: dynamic;
}

class an
{
  var name: dynamic;
  var whr: dynamic;
  var cl: dynamic;
  var val: dynamic;
  var num: dynamic;
}

var l: dynamic;

var anim: dynamic;

var d = cpp_array(3);

var used: dynamic;

var all_size: dynamic;

func Inputdata()
{
  read(n);
  l.resize(n);
  {
    var i = 0;
    while ((i < n))
    {
      var name: dynamic;
      var cls: dynamic;
      var a: dynamic;
      var b: dynamic;
      var c: dynamic;
      var s: dynamic;
      read(name, cls, a, b, c, s);
      l[i].name = name;
      l[i].size = s;
      all_size += s;
      if ((cls == "weapon"))
      {
        l[i].cl = 0;
        l[i].val = a;
      }
      if ((cls == "armor"))
      {
        l[i].cl = 1;
        l[i].val = b;
      }
      if ((cls == "orb"))
      {
        l[i].cl = 2;
        l[i].val = c;
      }
      i += 1;
    }
  }
  read(m);
  anim.resize(m);
  used.resize(m);
  {
    var i = 0;
    while ((i < m))
    {
      var name: dynamic;
      var type_cpp: dynamic;
      var whr: dynamic;
      var val: dynamic;
      read(name, type_cpp, val, whr);
      anim[i].name = name;
      anim[i].whr = whr;
      anim[i].val = val;
      anim[i].num = i;
      if ((type_cpp == "gladiator"))
      {
        anim[i].cl = 0;
      }
      if ((type_cpp == "sentry"))
      {
        anim[i].cl = 1;
      }
      if ((type_cpp == "physician"))
      {
        anim[i].cl = 2;
      }
      d[anim[i].cl].push_back(anim[i]);
      i += 1;
    }
  }
}

func Comp(a: dynamic, b: dynamic)
{
  return (a.val > b.val);
}

func Solve()
{
  sort(d[0].begin(), d[0].end(), Comp);
  sort(d[1].begin(), d[1].end(), Comp);
  sort(d[2].begin(), d[2].end(), Comp);
  var ans = cpp_array(3);
  var max_val = cpp_array(3);
  {
    var i = 0;
    while ((i < 3))
    {
      max_val[i] = -10;
      i += 1;
    }
  }
  if ((all_size > m))
  {
    {
      var i = 0;
      while ((i < n))
      {
        var type_cpp = l[i].cl;
        var now_val = l[i].val;
        {
          var j = 0;
          while ((j < min(l[i].size, int_cpp(d[type_cpp].size()))))
          {
            now_val += d[type_cpp][j].val;
            j += 1;
          }
        }
        if ((max_val[type_cpp] < now_val))
        {
          max_val[type_cpp] = now_val;
          ans[type_cpp] = i;
        }
        i += 1;
      }
    }
    var ans_out = cpp_array(3);
    {
      var i = 0;
      while ((i < 3))
      {
        {
          var j = 0;
          while ((j < min(l[ans[i]].size, int_cpp(d[i].size()))))
          {
            ans_out[i].push_back(d[i][j].name);
            used[d[i][j].num] = true;
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < 3))
      {
        var cnt = int_cpp(ans_out[i].size());
        {
          var j = 0;
          while ((j < m))
          {
            if ((cnt >= l[ans[i]].size))
            {
              break;
            }
            if ((!used[j]))
            {
              ans_out[i].push_back(anim[j].name);
              used[j] = true;
              cnt += 1;
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < 3))
      {
        write(l[ans[i]].name, cpp_char(" "));
        write(ans_out[i].size(), cpp_char(" "));
        {
          var j = 0;
          while ((j < ans_out[i].size()))
          {
            write(ans_out[i][j], cpp_char(" "));
            j += 1;
          }
        }
        write("\n");
        i += 1;
      }
    }
  } else
  {
    var max_val = cpp_array(3);
    var ans = cpp_array(3);
    {
      var i = 0;
      while ((i < n))
      {
        var type_cpp = l[i].cl;
        var now_val = l[i].val;
        {
          var j = 0;
          while ((j < m))
          {
            if (((anim[j].whr == l[i].name) && (l[i].cl == anim[j].cl)))
            {
              now_val += anim[j].val;
            }
            j += 1;
          }
        }
        if ((max_val[type_cpp] < now_val))
        {
          max_val[type_cpp] = now_val;
          ans[type_cpp] = i;
        }
        i += 1;
      }
    }
    var ans_out = cpp_array(3);
    {
      var i = 0;
      while ((i < 3))
      {
        var type_cpp = l[ans[i]].cl;
        {
          var j = 0;
          while ((j < m))
          {
            if ((anim[j].whr == l[ans[i]].name))
            {
              ans_out[i].push_back(anim[j].name);
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < 3))
      {
        write(l[ans[i]].name, cpp_char(" "));
        write(ans_out[i].size(), cpp_char(" "));
        {
          var j = 0;
          while ((j < ans_out[i].size()))
          {
            write(ans_out[i][j], cpp_char(" "));
            j += 1;
          }
        }
        write("\n");
        i += 1;
      }
    }
  }
}

func main()
{
  Inputdata();
  Solve();
  return 0;
}

// Translated from solution.cpp.

func main() -> dynamic
{
  var test: dynamic = cpp_uninitialized();
  read(test);
  var name: dynamic = cpp_array(50, 1010);
  var num: dynamic = cpp_array(1010);
  var mx: dynamic = 0;
  var m1: dynamic = cpp_uninitialized();
  var m2: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < test))
    {
      read(name[i], num[i]);
      m1[name[i]] += num[i];
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < test))
    {
      mx = max(mx, m1[name[i]]);
      i += 1;
    }
  }
  var j: dynamic = cpp_uninitialized();
  {
    j = 0;
    while ((j < test))
    {
      m2[name[j]] += num[j];
      if (((m1[name[j]] == mx) && (m2[name[j]] >= mx)))
      {
        break;
      }
      j += 1;
    }
  }
  write(name[j], "\n");
}

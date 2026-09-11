// Translated from solution.cpp.

func token(a: dynamic) -> dynamic
{
  var w: dynamic = cpp_uninitialized();
  a.push_back(cpp_char(" "));
  while ((!a.empty()))
  {
    w.push_back(a.substr(0, a.find(" ")));
    a = a.substr((a.find(" ") + 1), (a.size() - 1));
  }
  return w;
}

var mapik: dynamic = cpp_uninitialized();

var amapik: dynamic = cpp_uninitialized();

func dodaj(a: dynamic) -> dynamic
{
  if ((mapik.count(a) == 0))
  {
    mapik[a] = (mapik.size() - 1);
    amapik.push_back(a);
  }
  return mapik[a];
}

var tmp_str: dynamic = cpp_array(1000);

func scanf_string() -> dynamic
{
  scanf("%s", tmp_str);
  return tmp_str;
}

var N: dynamic = 1000;

var n: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%d", (&n));
  var inches: dynamic = (((n + 1)) / 3);
  printf("%d %d\n", (inches / 12), (inches % 12));
  return 0;
}

namespace builtin {

inline auto print = [](model::Object* self, const model::List* args) -> model::Object* {
    std::string text;
    for (const auto* arg : args->val) {
        text += arg->to_string() + " ";
    }
    std::cout << text << std::endl;
    return new model::Nil();
};

inline auto input = [](model::Object* self, const model::List* args) -> model::Object* {
    const auto prompt_obj = get_one_arg(args);
    std::cout << prompt_obj->to_string();
    std::string result;
    std::getline(std::cin, result);
    return new model::String(result);
};

inline auto isinstance = [](model::Object* self, const model::List* args) -> model::Object* {
    if (!(args->val.size() == 2)) {
        assert(false && "函数参数不足两个");
    }

    const auto a = args->val[0];
    const auto b = args->val[1];
    return check_based_object(a, b);
    
};

inline auto help = [](model::Object* self, const model::List* args) -> model::Object* {

};

inline auto breakpointer = [](model::Object* self, const model::List* args) -> model::Object* {

};

inline auto range = [](model::Object* self, const model::List* args) -> model::Object* {

};

inline auto cmd = [](model::Object* self, const model::List* args) -> model::Object* {

};

inline auto now = [](model::Object* self, const model::List* args) -> model::Object* {

};

inline auto setattr = [](model::Object* self, const model::List* args) -> model::Object* {

};

inline auto getattr = [](model::Object* self, const model::List* args) -> model::Object* {

};

inline auto delattr = [](model::Object* self, const model::List* args) -> model::Object* {

};

inline auto locals = [](model::Object* self, const model::List* args) -> model::Object* {

};

inline auto globals = [](model::Object* self, const model::List* args) -> model::Object* {

};

inline auto getrefc = [](model::Object* self, const model::List* args) -> model::Object* {

};

inline auto copy = [](model::Object* self, const model::List* args) -> model::Object* {

};

inline auto create = [](model::Object* self, const model::List* args) -> model::Object* {

};

inline auto typeof = [](model::Object* self, const model::List* args) -> model::Object* {

};

}
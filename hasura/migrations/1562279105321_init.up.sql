SET xmloption = content;
CREATE TABLE public.companys (
    id uuid DEFAULT public.gen_random_uuid() NOT NULL,
    name text NOT NULL,
    description text NOT NULL
);
CREATE TABLE public.users (
    id uuid DEFAULT public.gen_random_uuid() NOT NULL,
    first_name text NOT NULL,
    last_name text NOT NULL,
    email text NOT NULL,
    password_hash text NOT NULL,
    company_id uuid
);
ALTER TABLE ONLY public.companys
    ADD CONSTRAINT companys_pkey PRIMARY KEY (id);
ALTER TABLE ONLY public.users
    ADD CONSTRAINT users_email_key UNIQUE (email);
ALTER TABLE ONLY public.users
    ADD CONSTRAINT users_pkey PRIMARY KEY (id);
ALTER TABLE ONLY public.users
    ADD CONSTRAINT "users_companyId_fkey" FOREIGN KEY (company_id) REFERENCES public.companys(id) ON UPDATE SET NULL ON DELETE SET NULL;
